// AirMonitor V4: ESP8266 NodeMCU + 21VOC + PMS7003M + HF035 sGUI UART screen
// Screen: 212x318 HF035-QVGA, UART 115200, Host_TX = NodeMCU TX / GPIO1
// In sGUI make dynamic TEXT widgets with IDs:
//   0 PM2.5, 2 PM1.0, 3 PM10, 4 VOC, 5 HCHO,
//   6 eCO2, 7 temperature, 8 relative humidity, 9 status.
// ID 1 is reserved for the existing static PM2.5 title, if present.
// Keep all other fixed labels outside the dynamic ID set. No text debugging
// on Serial: UART0 TX is connected to the display.

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>

constexpr uint8_t VOC_PIN = D6;  // ESP8266 GPIO12, sensor TX -> RX
constexpr uint8_t PMS_PIN = D5;  // ESP8266 GPIO14, sensor TX -> RX
constexpr uint32_t STALE_MS = 5000;
constexpr uint32_t HMI_COMMAND_SPACING_MS = 260;

SoftwareSerial vocSerial(VOC_PIN, -1);  // RX, TX unused
SoftwareSerial pmsSerial(PMS_PIN, -1);  // RX, TX unused

struct VOCReading {
  uint16_t voc = 0, hcho = 0, eco2 = 0;
  int16_t temp10 = 0, rh10 = 0;
  uint32_t timestamp = 0;
  bool valid = false;
} gas;

struct PMSReading {
  uint16_t pm1 = 0, pm25 = 0, pm10 = 0;
  uint32_t timestamp = 0;
  bool valid = false;
} pm;

uint8_t vb[12] = {};
uint8_t pb[32] = {};
uint8_t vc = 0;
uint8_t pc = 0;
uint32_t vocOK = 0, vocBad = 0, pmsOK = 0, pmsBad = 0;
uint32_t lastHmi = 0;
uint8_t nextField = 0;
// Edit these IDs if sGUI assigned different IDs to your data text widgets.
const uint8_t HMI_TEXT_IDS[9] = {0, 2, 3, 4, 5, 6, 7, 8, 9};

uint16_t u16(const uint8_t* p) {
  return (uint16_t(p[0]) << 8) | p[1];
}

bool fresh(bool valid, uint32_t timestamp) {
  return valid && (uint32_t)(millis() - timestamp) < STALE_MS;
}

void feedVOC(uint8_t b) {
  if (vc == 0 && b != 0x2C) return;
  vb[vc++] = b;
  if (vc != 12) return;

  uint8_t sum = 0;
  for (uint8_t i = 0; i < 12; ++i) sum += vb[i];
  if (sum == 0) {
    gas.voc = u16(vb + 1);
    gas.hcho = u16(vb + 3);
    gas.eco2 = u16(vb + 5);
    gas.temp10 = (int16_t)u16(vb + 7);
    gas.rh10 = (int16_t)u16(vb + 9);
    gas.timestamp = millis();
    gas.valid = true;
    ++vocOK;
    vc = 0;
    return;
  }

  ++vocBad;
  uint8_t start = 1;
  while (start < 12 && vb[start] != 0x2C) ++start;
  if (start == 12) {
    vc = 0;
  } else {
    vc = 12 - start;
    memmove(vb, vb + start, vc);
  }
}

void resyncPMS() {
  uint8_t start = 1;
  while (start < pc) {
    if (pb[start] == 0x42 &&
        (start == pc - 1 || pb[start + 1] == 0x4D)) break;
    ++start;
  }
  if (start == pc) {
    pc = 0;
  } else {
    pc -= start;
    memmove(pb, pb + start, pc);
  }
}

void feedPMS(uint8_t b) {
  if (pc == 0 && b != 0x42) return;
  pb[pc++] = b;
  if (pc == 2 && pb[1] != 0x4D) {
    resyncPMS();
    return;
  }
  if (pc == 4 && u16(pb + 2) != 28) {
    ++pmsBad;
    resyncPMS();
    return;
  }
  if (pc < 32) return;

  uint16_t sum = 0;
  for (uint8_t i = 0; i < 30; ++i) sum += pb[i];
  if (sum == u16(pb + 30)) {
    pm.pm1 = u16(pb + 10);   // atmospheric PM1.0
    pm.pm25 = u16(pb + 12);  // atmospheric PM2.5
    pm.pm10 = u16(pb + 14);  // atmospheric PM10
    pm.timestamp = millis();
    pm.valid = true;
    ++pmsOK;
    pc = 0;
  } else {
    ++pmsBad;
    resyncPMS();
  }
}

void pollSensors() {
  while (vocSerial.available()) feedVOC((uint8_t)vocSerial.read());
  while (pmsSerial.available()) feedPMS((uint8_t)pmsSerial.read());
}

void formatTenths(char* dst, size_t size, int16_t raw) {
  int v = raw;
  unsigned int magnitude = (unsigned int)(v < 0 ? -v : v);
  snprintf(dst, size, "%s%u.%u", v < 0 ? "-" : "",
           magnitude / 10, magnitude % 10);
}

// One nonblocking UI update at a time. Allow ~260ms before the next.
// This gives each SET_TXT command time to execute on the screen.
void sendNextField() {
  const bool gOK = fresh(gas.valid, gas.timestamp);
  const bool pOK = fresh(pm.valid, pm.timestamp);
  char value[30] = "--";

  switch (nextField) {
    case 0: if (pOK) snprintf(value, sizeof(value), "%u", pm.pm25); break;
    case 1: if (pOK) snprintf(value, sizeof(value), "%u", pm.pm1); break;
    case 2: if (pOK) snprintf(value, sizeof(value), "%u", pm.pm10); break;
    case 3: if (gOK) snprintf(value, sizeof(value), "%u", gas.voc); break;
    case 4: if (gOK) snprintf(value, sizeof(value), "%u", gas.hcho); break;
    case 5: if (gOK) snprintf(value, sizeof(value), "%u", gas.eco2); break;
    case 6: if (gOK) formatTenths(value, sizeof(value), gas.temp10); break;
    case 7: if (gOK) formatTenths(value, sizeof(value), gas.rh10); break;
    case 8:
      snprintf(value, sizeof(value), "%s",
               (gOK && pOK) ? "ONLINE" :
               (!gOK && !pOK) ? "WAIT" : (!gOK ? "VOC ERR" : "PMS ERR"));
      break;
  }

  // HF-series command: SET_TXT(id,'text');\r\n
  Serial.print("SET_TXT(");
  Serial.print(HMI_TEXT_IDS[nextField]);
  Serial.print(",'" );
  Serial.print(value);
  Serial.print("');\r\n");
  nextField = (nextField + 1) % 9;
}

void setup() {
  Serial.begin(115200);  // LCD HMI on GPIO1 (UART0 TX)
  Serial.setDebugOutput(false);
  vocSerial.begin(9600);
  pmsSerial.begin(9600);

  // Wait for the display to finish booting before sending commands.
  delay(1500);
  Serial.print("BL(180);\r\n");
  lastHmi = millis();
}

void loop() {
  pollSensors();
  const uint32_t now = millis();
  if ((uint32_t)(now - lastHmi) >= HMI_COMMAND_SPACING_MS) {
    lastHmi = now;
    sendNextField();
  }
  yield();
}
