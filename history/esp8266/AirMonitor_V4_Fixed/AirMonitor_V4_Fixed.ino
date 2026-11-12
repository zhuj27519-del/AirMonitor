/*
  AirMonitor V4 - ESP8266 NodeMCU + 21VOC + PMS7003M + HF035 UART HMI
  ESP8266 core 3.1.2 / EspSoftwareSerial 8.x

  21VOC TX -> D6 (GPIO12), 9600 8N1
  PMS7003M TX -> D5 (GPIO14), 9600 8N1
  HF035 HMI host-labelled TX -> NodeMCU TX (GPIO1), 115200 8N1
  All grounds common. Power the 5V screen and PMS7003M from a suitable 5V supply.

  sGUI text IDs: 0=PM2.5, 1=static title (not updated), 2=PM1,
  3=PM10, 4=VOC, 5=HCHO, 6=eCO2, 7=TEMP, 8=RH, 9=STATUS.
*/
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>

static const uint8_t VOC_RX_PIN = D6;
static const uint8_t PMS_RX_PIN = D5;
static const uint32_t SENSOR_BAUD = 9600;
static const uint32_t HMI_BAUD = 115200;
static const uint32_t STALE_MS = 5000;
static const uint32_t DISPLAY_STEP_MS = 260;
static const uint8_t TEXT_IDS[9] = {0, 2, 3, 4, 5, 6, 7, 8, 9};

// TX is unused for both sensors. ESP8266 EspSoftwareSerial supports RX-only instances.
SoftwareSerial vocPort(VOC_RX_PIN, -1);
SoftwareSerial pmsPort(PMS_RX_PIN, -1);

struct VocReading {
  uint16_t voc = 0;
  uint16_t hcho = 0;
  uint16_t eco2 = 0;
  int16_t tempTenth = 0;
  int16_t rhTenth = 0;
  uint32_t updatedAt = 0;
  bool received = false;
};
struct PmReading {
  uint16_t pm1 = 0;
  uint16_t pm25 = 0;
  uint16_t pm10 = 0;
  uint32_t updatedAt = 0;
  bool received = false;
};
VocReading gas;
PmReading pm;

uint8_t vocFrame[12];
uint8_t vocLen = 0;
uint8_t pmsFrame[32];
uint8_t pmsLen = 0;
uint32_t vocValid = 0, vocBad = 0, pmsValid = 0, pmsBad = 0;

// Avoid name 'u16': it is already defined as an SDK typedef on ESP8266.
uint16_t readBE16(const uint8_t *p) {
  return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}

void resyncVoc() {
  uint8_t start = 1;
  while (start < vocLen && vocFrame[start] != 0x2C) ++start;
  if (start >= vocLen) {
    vocLen = 0;
  } else {
    vocLen -= start;
    memmove(vocFrame, vocFrame + start, vocLen);
  }
}

void feedVoc(uint8_t b) {
  if (vocLen == 0 && b != 0x2C) return;
  vocFrame[vocLen++] = b;
  if (vocLen < 12) return;

  uint8_t checksum = 0;
  for (uint8_t i = 0; i < 12; i++) checksum += vocFrame[i];
  if (checksum != 0) {
    ++vocBad;
    resyncVoc();
    return;
  }

  gas.voc = readBE16(vocFrame + 1);
  gas.hcho = readBE16(vocFrame + 3);
  gas.eco2 = readBE16(vocFrame + 5);
  gas.tempTenth = static_cast<int16_t>(readBE16(vocFrame + 7));
  gas.rhTenth = static_cast<int16_t>(readBE16(vocFrame + 9));
  gas.updatedAt = millis();
  gas.received = true;
  ++vocValid;
  vocLen = 0;
}

void resyncPms() {
  uint8_t start = 1;
  while (start < pmsLen) {
    if (pmsFrame[start] == 0x42 &&
        (start == pmsLen - 1 || pmsFrame[start + 1] == 0x4D)) break;
    ++start;
  }
  if (start >= pmsLen) {
    pmsLen = 0;
  } else {
    pmsLen -= start;
    memmove(pmsFrame, pmsFrame + start, pmsLen);
  }
}

void feedPms(uint8_t b) {
  if (pmsLen == 0 && b != 0x42) return;
  pmsFrame[pmsLen++] = b;

  if (pmsLen == 2 && pmsFrame[1] != 0x4D) {
    resyncPms();
    return;
  }
  if (pmsLen == 4 && readBE16(pmsFrame + 2) != 28) {
    ++pmsBad;
    resyncPms();
    return;
  }
  if (pmsLen < 32) return;

  uint16_t checksum = 0;
  for (uint8_t i = 0; i < 30; ++i) checksum += pmsFrame[i];
  if (checksum != readBE16(pmsFrame + 30)) {
    ++pmsBad;
    resyncPms();
    return;
  }

  // PMS7003 atmospheric-environment mass concentrations (not CF=1).
  pm.pm1 = readBE16(pmsFrame + 10);
  pm.pm25 = readBE16(pmsFrame + 12);
  pm.pm10 = readBE16(pmsFrame + 14);
  pm.updatedAt = millis();
  pm.received = true;
  ++pmsValid;
  pmsLen = 0;
}

void pollSensors() {
  while (vocPort.available() > 0) {
    int b = vocPort.read();
    if (b >= 0) feedVoc(static_cast<uint8_t>(b));
  }
  while (pmsPort.available() > 0) {
    int b = pmsPort.read();
    if (b >= 0) feedPms(static_cast<uint8_t>(b));
  }
}

bool fresh(bool received, uint32_t updatedAt) {
  return received && (uint32_t)(millis() - updatedAt) < STALE_MS;
}

// Text controls should contain short ASCII strings. Fixed units/labels belong in sGUI.
void sendText(uint8_t id, const char *value) {
  Serial.print(F("SET_TXT("));
  Serial.print(id);
  Serial.print(F(",'"));
  Serial.print(value);
  Serial.print(F("');\r\n"));
}

void formatTenth(char *out, size_t n, int16_t raw) {
  // snprintf float formatting is not supported by every embedded build.
  long v = raw;
  bool neg = (v < 0);
  if (neg) v = -v;
  snprintf(out, n, "%s%ld.%ld", neg ? "-" : "", v / 10, v % 10);
}

void updateOneText(uint8_t index) {
  char value[24] = "--";
  bool pmOK = fresh(pm.received, pm.updatedAt);
  bool vocOK = fresh(gas.received, gas.updatedAt);

  switch (index) {
    case 0: if (pmOK) snprintf(value, sizeof(value), "%u", pm.pm25); break;
    case 1: if (pmOK) snprintf(value, sizeof(value), "%u", pm.pm1); break;
    case 2: if (pmOK) snprintf(value, sizeof(value), "%u", pm.pm10); break;
    case 3: if (vocOK) snprintf(value, sizeof(value), "%u", gas.voc); break;
    case 4: if (vocOK) snprintf(value, sizeof(value), "%u", gas.hcho); break;
    case 5: if (vocOK) snprintf(value, sizeof(value), "%u", gas.eco2); break;
    case 6: if (vocOK) formatTenth(value, sizeof(value), gas.tempTenth); break;
    case 7: if (vocOK) formatTenth(value, sizeof(value), gas.rhTenth); break;
    case 8:
      if (pmOK && vocOK) strcpy(value, "ONLINE");
      else if (!pmOK && !vocOK) strcpy(value, "WAIT");
      else if (!pmOK) strcpy(value, "PMS ERR");
      else strcpy(value, "VOC ERR");
      break;
  }
  sendText(TEXT_IDS[index], value);
}

void setup() {
  // On NodeMCU, UART0 TX = GPIO1. Keep ordinary Serial diagnostics disabled.
  Serial.begin(HMI_BAUD);
  vocPort.begin(SENSOR_BAUD);
  pmsPort.begin(SENSOR_BAUD);
  delay(1500);  // Let HF035 finish booting.
}

void loop() {
  pollSensors();
  static uint8_t nextText = 0;
  static uint32_t lastTextAt = 0;
  if ((uint32_t)(millis() - lastTextAt) >= DISPLAY_STEP_MS) {
    lastTextAt = millis();
    updateOneText(nextText);
    nextText = (nextText + 1) % 9;
  }
  yield();
}
