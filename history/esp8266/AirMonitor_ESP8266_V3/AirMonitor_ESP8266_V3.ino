/*
  AirMonitor V4 -- HF035 212x318 / 9 sGUI text fields
  Board: NodeMCU 1.0 (ESP-12E Module), ESP8266 core 3.1.2

  Screenshot-confirmed Page0 text IDs:
    0 PM2.5       1 PM1.0      2 VOC
    3 Humidity    4 Status     5 eCO2
    6 PM10        7 Temp       8 HCHO

  Wiring (keep the already tested arrangement):
    21VOC TX             -> D6 (GPIO12), UART 9600 8N1
    PMS7003M TX          -> D5 (GPIO14), UART 9600 8N1
    HF035 host-side 'TX' -> NodeMCU TX (GPIO1), UART 115200 8N1
    All grounds connected; LCD and PMS use suitable 5V supply.

  sGUI Page0: text controls must have exactly IDs 0..8.
  Static labels and measurement units are contained in the background BMP.
  Keep Serial reserved for HF035 commands: no Serial debug printing.
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <stdio.h>

constexpr uint32_t SENSOR_BAUD = 9600;
constexpr uint32_t HMI_BAUD = 115200;
constexpr uint32_t STALE_AFTER_MS = 5000;
constexpr uint32_t SEND_INTERVAL_MS = 250;

SoftwareSerial vocUart(D6, -1);  // receive only
SoftwareSerial pmsUart(D5, -1);  // receive only

struct VocData {
  uint16_t voc = 0;
  uint16_t hcho = 0;
  uint16_t eco2 = 0;
  int16_t temp10 = 0;
  int16_t humidity10 = 0;
  uint32_t lastMs = 0;
  bool seen = false;
};

struct PmsData {
  uint16_t pm1 = 0;
  uint16_t pm25 = 0;
  uint16_t pm10 = 0;
  uint32_t lastMs = 0;
  bool seen = false;
};

VocData voc;
PmsData pms;

uint8_t vocFrame[12];
uint8_t vocLen = 0;
uint8_t pmsFrame[32];
uint8_t pmsLen = 0;

// ESP8266 already has a type called u16; avoid naming functions u16().
uint16_t be16(const uint8_t *bytes) {
  return (uint16_t(bytes[0]) << 8) | bytes[1];
}

// 21VOC: header 0x2C, 12 bytes, unsigned 8-bit sum == 0.
void alignVoc() {
  uint8_t start = 1;
  while (start < vocLen && vocFrame[start] != 0x2C) ++start;
  if (start == vocLen) {
    vocLen = 0;
  } else {
    vocLen -= start;
    memmove(vocFrame, vocFrame + start, vocLen);
  }
}

void consumeVoc(uint8_t b) {
  if (vocLen == 0 && b != 0x2C) return;
  vocFrame[vocLen++] = b;
  if (vocLen != sizeof(vocFrame)) return;

  uint8_t checksum = 0;
  for (uint8_t i = 0; i < sizeof(vocFrame); ++i) checksum += vocFrame[i];
  if (checksum != 0) {
    alignVoc();
    return;
  }

  voc.voc        = be16(vocFrame + 1);
  voc.hcho       = be16(vocFrame + 3);
  voc.eco2       = be16(vocFrame + 5);
  voc.temp10     = (int16_t)be16(vocFrame + 7);
  voc.humidity10 = (int16_t)be16(vocFrame + 9);
  voc.lastMs     = millis();
  voc.seen       = true;
  vocLen         = 0;
}

// PMS7003M: 42 4D, 0x001C length, total 32 bytes, 16-bit sum.
void alignPms() {
  uint8_t start = 1;
  while (start < pmsLen) {
    if (pmsFrame[start] == 0x42 &&
        (start == pmsLen - 1 || pmsFrame[start + 1] == 0x4D)) break;
    ++start;
  }
  if (start == pmsLen) {
    pmsLen = 0;
  } else {
    pmsLen -= start;
    memmove(pmsFrame, pmsFrame + start, pmsLen);
  }
}

void consumePms(uint8_t b) {
  if (pmsLen == 0 && b != 0x42) return;
  pmsFrame[pmsLen++] = b;

  if (pmsLen == 2 && pmsFrame[1] != 0x4D) {
    alignPms();
    return;
  }
  if (pmsLen == 4 && be16(pmsFrame + 2) != 28) {
    alignPms();
    return;
  }
  if (pmsLen != sizeof(pmsFrame)) return;

  uint16_t checksum = 0;
  for (uint8_t i = 0; i < 30; ++i) checksum += pmsFrame[i];
  if (checksum != be16(pmsFrame + 30)) {
    alignPms();
    return;
  }

  // Atmospheric PM values, not factory CF=1 values.
  pms.pm1    = be16(pmsFrame + 10);
  pms.pm25   = be16(pmsFrame + 12);
  pms.pm10   = be16(pmsFrame + 14);
  pms.lastMs = millis();
  pms.seen   = true;
  pmsLen     = 0;
}

void pollSensors() {
  while (vocUart.available()) {
    int b = vocUart.read();
    if (b >= 0) consumeVoc((uint8_t)b);
  }
  while (pmsUart.available()) {
    int b = pmsUart.read();
    if (b >= 0) consumePms((uint8_t)b);
  }
}

bool recent(bool seen, uint32_t t) {
  return seen && (uint32_t)(millis() - t) < STALE_AFTER_MS;
}

// Send a complete HF035 sGUI command, e.g. SET_TXT(6,'31');\r\n.
void setText(uint8_t id, const char *s) {
  Serial.print(F("SET_TXT("));
  Serial.print(id);
  Serial.print(F(",'"));
  Serial.print(s);
  Serial.print(F("');\r\n"));
}

// Format signed sensor tenths without float printf support.
void tenthToText(char *dst, size_t capacity, int16_t ten) {
  long v = ten;
  bool minus = v < 0;
  if (minus) v = -v;
  snprintf(dst, capacity, "%s%ld.%ld", minus ? "-" : "", v / 10, v % 10);
}

// Layout is based on the red numeric IDs in your latest sGUI screenshot.
void updateField(uint8_t id) {
  char value[20] = "--";
  const bool hasPms = recent(pms.seen, pms.lastMs);
  const bool hasVoc = recent(voc.seen, voc.lastMs);

  switch (id) {
    case 0: // Main PM2.5
      if (hasPms) snprintf(value, sizeof(value), "%u", pms.pm25);
      break;
    case 1: // PM1.0 upper left
      if (hasPms) snprintf(value, sizeof(value), "%u", pms.pm1);
      break;
    case 2: // VOC second-row left
      if (hasVoc) snprintf(value, sizeof(value), "%u", voc.voc);
      break;
    case 3: // Humidity third-row left
      if (hasVoc) tenthToText(value, sizeof(value), voc.humidity10);
      break;
    case 4: // Status bottom left
      if (hasPms && hasVoc) strcpy(value, "ONLINE");
      else if (!hasPms && !hasVoc) strcpy(value, "WAIT");
      else if (!hasPms) strcpy(value, "PMS ERR");
      else strcpy(value, "VOC ERR");
      break;
    case 5: // eCO2 second-row right
      if (hasVoc) snprintf(value, sizeof(value), "%u", voc.eco2);
      break;
    case 6: // PM10 upper right
      if (hasPms) snprintf(value, sizeof(value), "%u", pms.pm10);
      break;
    case 7: // Temperature third-row right
      if (hasVoc) tenthToText(value, sizeof(value), voc.temp10);
      break;
    case 8: // HCHO bottom right
      if (hasVoc) snprintf(value, sizeof(value), "%u", voc.hcho);
      break;
  }
  setText(id, value);
}

void setup() {
  // GPIO1 is the HF035 serial output; do not send debug strings to Serial.
  Serial.begin(HMI_BAUD);
  vocUart.begin(SENSOR_BAUD);
  pmsUart.begin(SENSOR_BAUD);
  delay(1500); // HF035 startup
}

void loop() {
  pollSensors();

  static uint32_t lastSent = 0;
  static uint8_t nextId = 0;
  if ((uint32_t)(millis() - lastSent) >= SEND_INTERVAL_MS) {
    lastSent = millis();
    updateField(nextId);
    nextId = (nextId + 1) % 9;
  }
  yield();
}

