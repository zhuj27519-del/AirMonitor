#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// NodeMCU ESP-12F wiring
// OLED SDA=D2(GPIO4), SCL=D1(GPIO5), power=3V3
// PMS7003M TX -> D5(GPIO14), power=verified 5V VU
// 21VOC TX -> ESP RX(GPIO3), power=3V3 only if rated for it
// Disconnect 21VOC TX/RX while uploading.

static const uint8_t OLED_SDA_PIN = D2;
static const uint8_t OLED_SCL_PIN = D1;
static const uint8_t PMS_RX_PIN = D5;
static const uint32_t PAGE_MS = 2500;
static const uint32_t DISPLAY_MS = 300;
static const uint32_t DATA_TIMEOUT_MS = 6000;

SoftwareSerial pms(PMS_RX_PIN, -1);
uint8_t oledAddress = 0;
bool oledReady = false;
uint8_t frameBuffer[128 * 8];

uint16_t pm1 = 0, pm25 = 0, pm10 = 0;
bool pmSeen = false;
uint32_t lastPM = 0;
uint32_t pmFrames = 0;

uint16_t tvocRaw = 0, hchoRaw = 0, eco2Raw = 0;
uint16_t tempRaw = 0, humRaw = 0;
bool vocSeen = false;
uint32_t lastVOC = 0;
uint32_t vocFrames = 0;
uint32_t vocBytes = 0;

// Limited built-in 5x7 font: '0'..'9', ':', '-', '.', '%', uppercase 'A'..'Z'.
const uint8_t font[][5] PROGMEM = {
 {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},
 {0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
 {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},
 {0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
 {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},
 {0x00,0x36,0x36,0x00,0x00},{0x08,0x08,0x08,0x08,0x08},
 {0x00,0x60,0x60,0x00,0x00},{0x63,0x13,0x08,0x64,0x63},
 {0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},
 {0x3E,0x41,0x41,0x41,0x22},{0x7F,0x41,0x41,0x22,0x1C},
 {0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},
 {0x3E,0x41,0x49,0x49,0x7A},{0x7F,0x08,0x08,0x08,0x7F},
 {0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},
 {0x7F,0x08,0x14,0x22,0x41},{0x7F,0x40,0x40,0x40,0x40},
 {0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},
 {0x3E,0x41,0x41,0x41,0x3E},{0x7F,0x09,0x09,0x09,0x06},
 {0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7F,0x01,0x01},
 {0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},
 {0x7F,0x20,0x18,0x20,0x7F},{0x63,0x14,0x08,0x14,0x63},
 {0x03,0x04,0x78,0x04,0x03},{0x61,0x51,0x49,0x45,0x43}
};

int glyph(char c) {
 if (c >= '0' && c <= '9') return c - '0';
 if (c == ':') return 10;
 if (c == '-') return 11;
 if (c == '.') return 12;
 if (c == '%') return 13;
 if (c >= 'a' && c <= 'z') c -= 32;
 if (c >= 'A' && c <= 'Z') return c - 'A' + 14;
 return -1;
}

void px(int x, int y) {
 if ((unsigned)x >= 128 || (unsigned)y >= 64) return;
 frameBuffer[x + (y / 8) * 128] |= (1u << (y & 7));
}

void textAt(int x, int y, const char *s) {
 while (*s && x <= 122) {
  int index = glyph(*s++);
  if (index >= 0) {
   for (int i = 0; i < 5; ++i) {
    uint8_t bits = pgm_read_byte(&font[index][i]);
    for (int j = 0; j < 7; ++j) if (bits & (1u << j)) px(x + i, y + j);
   }
  }
  x += 6;
 }
}

bool oledCmd(uint8_t value) {
 Wire.beginTransmission(oledAddress);
 Wire.write((uint8_t)0x00);
 Wire.write(value);
 return Wire.endTransmission() == 0;
}

bool beginOLED() {
 for (uint8_t a = 0x3C; a <= 0x3D; ++a) {
  Wire.beginTransmission(a);
  if (Wire.endTransmission() == 0) { oledAddress = a; break; }
 }
 if (!oledAddress) return false;
 // SSD1306 128x64, page addressing (0x20, 0x02)
 const uint8_t setup[] = {0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,0x40,
  0x8D,0x14,0x20,0x02,0xA1,0xC8,0xDA,0x12,0x81,0x7F,
  0xD9,0xF1,0xDB,0x40,0xA4,0xA6,0xAF};
 for (uint8_t c : setup) if (!oledCmd(c)) return false;
 return true;
}

void flushOLED() {
 if (!oledReady) return;
 for (int page = 0; page < 8; ++page) {
  if (!oledCmd(0xB0 + page) || !oledCmd(0x00) || !oledCmd(0x10)) return;
  for (int offset = 0; offset < 128; offset += 16) {
   Wire.beginTransmission(oledAddress);
   Wire.write((uint8_t)0x40);
   for (int n = 0; n < 16; ++n)
    Wire.write(frameBuffer[page * 128 + offset + n]);
   if (Wire.endTransmission() != 0) return;
  }
  yield();
 }
}

void readPMS() {
 static uint8_t bytes[32];
 static uint8_t pos = 0;
 while (pms.available()) {
  uint8_t b = (uint8_t)pms.read();
  if (pos == 0 && b != 0x42) continue;
  if (pos == 1 && b != 0x4D) { pos = (b == 0x42) ? 1 : 0; if (pos) bytes[0] = b; continue; }
  bytes[pos++] = b;
  if (pos == 4 && (bytes[2] != 0 || bytes[3] != 28)) { pos = 0; continue; }
  if (pos < 32) continue;
  pos = 0;
  uint16_t checksum = 0;
  for (int i = 0; i < 30; ++i) checksum += bytes[i];
  if (checksum != ((uint16_t)bytes[30] << 8 | bytes[31])) continue;
  pm1  = ((uint16_t)bytes[10] << 8) | bytes[11];
  pm25 = ((uint16_t)bytes[12] << 8) | bytes[13];
  pm10 = ((uint16_t)bytes[14] << 8) | bytes[15];
  pmSeen = true;
  lastPM = millis();
  ++pmFrames;
 }
}

// Candidate 21VOC protocol: 0x2C + 11 more bytes, sum of 12 bytes mod 256 == 0.
// Not verified for this physical module; raw values shown without asserted units.
void readVOC() {
 static uint8_t bytes[12];
 static uint8_t pos = 0;
 static uint32_t lastByte = 0;
 while (Serial.available()) {
  uint8_t b = (uint8_t)Serial.read();
  ++vocBytes;
  if (pos && millis() - lastByte > 100) pos = 0;
  lastByte = millis();
  if (pos == 0 && b != 0x2C) continue;
  bytes[pos++] = b;
  if (pos < 12) continue;
  pos = 0;
  uint8_t sum = 0;
  for (uint8_t v : bytes) sum += v;
  if (sum != 0) continue;
  tvocRaw = ((uint16_t)bytes[1] << 8) | bytes[2];
  hchoRaw = ((uint16_t)bytes[3] << 8) | bytes[4];
  eco2Raw = ((uint16_t)bytes[5] << 8) | bytes[6];
  tempRaw = ((uint16_t)bytes[7] << 8) | bytes[8];
  humRaw = ((uint16_t)bytes[9] << 8) | bytes[10];
  vocSeen = true;
  lastVOC = millis();
  ++vocFrames;
 }
}

void drawPage(uint32_t now) {
 if (!oledReady) return;
 memset(frameBuffer, 0, sizeof(frameBuffer));
 uint8_t page = (now / PAGE_MS) % 2;  // independent of both sensors
 char line[24];
 if (page == 0) {
  textAt(0, 0, "PAGE 1  PM DATA");
  if (pmSeen && now - lastPM < DATA_TIMEOUT_MS) {
   snprintf(line, sizeof(line), "PM1: %u", pm1);  textAt(0, 14, line);
   snprintf(line, sizeof(line), "PM25: %u", pm25); textAt(0, 26, line);
   snprintf(line, sizeof(line), "PM10: %u", pm10); textAt(0, 38, line);
  } else {
   textAt(0, 20, "PMS WAITING");
  }
  snprintf(line, sizeof(line), "PM FRAMES:%lu", (unsigned long)pmFrames);
  textAt(0, 50, line);
 } else {
  textAt(0, 0, "PAGE 2  VOC DATA");
  if (vocSeen && now - lastVOC < DATA_TIMEOUT_MS) {
   snprintf(line, sizeof(line), "TVOC:%u", tvocRaw); textAt(0, 11, line);
   snprintf(line, sizeof(line), "HCHO:%u", hchoRaw); textAt(0, 21, line);
   snprintf(line, sizeof(line), "ECO2:%u", eco2Raw); textAt(0, 31, line);
   snprintf(line, sizeof(line), "TEMP RAW:%u", tempRaw); textAt(0, 41, line);
   snprintf(line, sizeof(line), "HUM RAW:%u", humRaw); textAt(0, 51, line);
  } else {
   textAt(0, 20, "VOC WAITING");
   snprintf(line, sizeof(line), "UART BYTES:%lu", (unsigned long)vocBytes);
   textAt(0, 35, line);
  }
 }
 // RUN seconds is diagnostic. If repeatedly resets to zero, board restarts.
 snprintf(line, sizeof(line), "RUN:%lus", (unsigned long)(now / 1000));
 textAt(78, 57, line);
 flushOLED();
}

void setup() {
 Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
 Wire.setClock(100000);
 oledReady = beginOLED();
 pms.begin(9600);
 Serial.begin(9600); // 21VOC candidate baud; RX0 conflicts with USB-UART on some boards
 if (oledReady) {
  memset(frameBuffer, 0, sizeof(frameBuffer));
  textAt(0, 0, "AIR MONITOR");
  textAt(0, 20, "BOOT");
  flushOLED();
 }
}

void loop() {
 uint32_t now = millis();
 readPMS();
 readVOC();
 static uint32_t lastDisplay = 0;
 if ((uint32_t)(now - lastDisplay) >= DISPLAY_MS) {
  lastDisplay = now;
  drawPage(now);
 }
 yield();
}
