#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// NodeMCU ESP8266: OLED SDA=D2, SCL=D1; PMS TX -> D5
// 21VOC TX -> ESP RX (GPIO3), 9600 8N1
// Assumed OLED controller: SSD1306 128x64 I2C
#define OLED_SDA D2
#define OLED_SCL D1
#define PMS_RX D5

SoftwareSerial pms(PMS_RX, -1);
uint8_t oledAddress = 0;
bool oledReady = false;
uint8_t framebuffer[1024];

// 0-9, colon, minus, period, space, A-Z, percent
const uint8_t glyphs[][5] PROGMEM = {
  {0x3E,0x51,0x49,0x45,0x3E}, {0,0x42,0x7F,0x40,0},
  {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
  {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
  {0x3C,0x4A,0x49,0x49,0x30}, {1,0x71,9,5,3},
  {0x36,0x49,0x49,0x49,0x36}, {6,0x49,0x49,0x29,0x1E},
  {0,0x36,0x36,0,0}, {0,8,8,8,0}, {0,0x60,0x60,0,0},
  {0,0,0,0,0},
  {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
  {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
  {0x7F,0x49,0x49,0x49,0x41}, {0x7F,9,9,9,1},
  {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,8,8,8,0x7F},
  {0,0x41,0x7F,0x41,0}, {0x20,0x40,0x41,0x3F,1},
  {0x7F,8,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
  {0x7F,2,0x0C,2,0x7F}, {0x7F,4,8,0x10,0x7F},
  {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,9,9,9,6},
  {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,9,0x19,0x29,0x46},
  {0x46,0x49,0x49,0x49,0x31}, {1,1,0x7F,1,1},
  {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
  {0x7F,0x20,0x18,0x20,0x7F}, {0x63,0x14,8,0x14,0x63},
  {3,4,0x78,4,3}, {0x61,0x51,0x49,0x45,0x43},
  {0x23,0x13,8,0x64,0x62}
};

int charIndex(char c) {
  if (c >= '0' && c <= '9') return c-'0';
  if (c == ':') return 10;
  if (c == '-') return 11;
  if (c == '.') return 12;
  if (c == ' ') return 13;
  if (c >= 'a' && c <= 'z') c -= 32;
  if (c >= 'A' && c <= 'Z') return 14 + c-'A';
  if (c == '%') return 40;
  return 13;
}

void dot(int x, int y) {
  if (x>=0 && x<128 && y>=0 && y<64)
    framebuffer[(y>>3)*128+x] |= (1 << (y&7));
}
void textAt(int x, int y, const char *s) {
  while (*s && x<=122) {
    int n=charIndex(*s++);
    for (int a=0;a<5;a++) {
      uint8_t bits=pgm_read_byte(&glyphs[n][a]);
      for (int b=0;b<7;b++) if (bits & (1<<b)) dot(x+a,y+b);
    }
    x+=6;
  }
}
void oledCmd(uint8_t b) {
  Wire.beginTransmission(oledAddress);
  Wire.write(0);
  Wire.write(b);
  Wire.endTransmission();
}
void oledFlush() {
  if (!oledReady) return;
  for (int page=0;page<8;page++) {
    oledCmd(0xB0+page);
    oledCmd(0x00);
    oledCmd(0x10);
    for (int offset=0;offset<128;offset+=16) {
      Wire.beginTransmission(oledAddress);
      Wire.write(0x40);
      for (int i=0;i<16;i++) Wire.write(framebuffer[page*128+offset+i]);
      Wire.endTransmission();
    }
  }
}
void oledStart() {
  Wire.begin(OLED_SDA,OLED_SCL);
  Wire.setClock(100000);
  for (uint8_t a=0x3C;a<=0x3D;a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission()==0) { oledAddress=a; break; }
  }
  if (!oledAddress) return;
  const uint8_t cmds[]={
    0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0,0x40,0x8D,0x14,
    0x20,0x02,0xA1,0xC8,0xDA,0x12,0x81,0x7F,
    0xD9,0xF1,0xDB,0x40,0xA4,0xA6,0xAF
  };
  for (uint8_t b : cmds) oledCmd(b);
  oledReady=true;
  memset(framebuffer,0,sizeof(framebuffer));
  textAt(0,15,"AIR MONITOR");
  textAt(0,30,"STARTING");
  oledFlush();
}

uint16_t pm1=0,pm25=0,pm10=0;
unsigned long lastPM=0;
uint8_t pmFrame[32];
uint8_t pmIndex=0;
void pollPMS() {
  while (pms.available()) {
    uint8_t b=pms.read();
    if (pmIndex==0 && b!=0x42) continue;
    if (pmIndex==1 && b!=0x4D) {
      pmIndex=0;
      if (b==0x42) { pmFrame[0]=b; pmIndex=1; }
      continue;
    }
    pmFrame[pmIndex++]=b;
    if (pmIndex==4 && (pmFrame[2]!=0 || pmFrame[3]!=28)) {
      pmIndex=0; continue;
    }
    if (pmIndex==32) {
      pmIndex=0;
      uint16_t sum=0;
      for (int i=0;i<30;i++) sum+=pmFrame[i];
      if (sum!=(((uint16_t)pmFrame[30]<<8)|pmFrame[31])) continue;
      pm1=((uint16_t)pmFrame[10]<<8)|pmFrame[11];
      pm25=((uint16_t)pmFrame[12]<<8)|pmFrame[13];
      pm10=((uint16_t)pmFrame[14]<<8)|pmFrame[15];
      lastPM=millis();
    }
  }
}

// Assumes 21VOC/JW01 UART frame:
// [0]=2C, [1..2]=TVOC ug/m3, [3..4]=HCHO ug/m3,
// [5..6]=eCO2 ppm, [7..8]=temperature x10 signed,
// [9..10]=RH x10, [11]=negative sum checksum
uint16_t tvoc=0,hcho=0,eco2=0,humid10=0;
int16_t temp10=0;
unsigned long lastVOC=0;
uint8_t vocFrame[12];
uint8_t vocIndex=0;
void pollVOC() {
  while (Serial.available()) {
    uint8_t b=(uint8_t)Serial.read();
    if (vocIndex==0 && b!=0x2C) continue;
    vocFrame[vocIndex++]=b;
    if (vocIndex==12) {
      vocIndex=0;
      uint8_t sum=0;
      for (uint8_t i=0;i<12;i++) sum+=vocFrame[i];
      if (sum!=0) continue;
      tvoc=((uint16_t)vocFrame[1]<<8)|vocFrame[2];
      hcho=((uint16_t)vocFrame[3]<<8)|vocFrame[4];
      eco2=((uint16_t)vocFrame[5]<<8)|vocFrame[6];
      temp10=(int16_t)(((uint16_t)vocFrame[7]<<8)|vocFrame[8]);
      humid10=((uint16_t)vocFrame[9]<<8)|vocFrame[10];
      lastVOC=millis();
    }
  }
}

void render() {
  if (!oledReady) return;
  memset(framebuffer,0,sizeof(framebuffer));
  char s[32];
  if ((millis()/4000)%2==0) {
    textAt(0,0,"AIR MONITOR");
    textAt(0,11,"PM UG M3");
    if (lastPM && millis()-lastPM<5000) {
      snprintf(s,sizeof(s),"PM1: %u",pm1); textAt(0,23,s);
      snprintf(s,sizeof(s),"PM25: %u",pm25); textAt(0,35,s);
      snprintf(s,sizeof(s),"PM10: %u",pm10); textAt(0,47,s);
    } else textAt(0,28,"PMS WAITING");
  } else {
    textAt(0,0,"VOC SENSOR");
    if (lastVOC && millis()-lastVOC<12000) {
      snprintf(s,sizeof(s),"TVOC: %u",tvoc); textAt(0,12,s);
      snprintf(s,sizeof(s),"HCHO: %u",hcho); textAt(0,23,s);
      snprintf(s,sizeof(s),"ECO2: %u",eco2); textAt(0,34,s);
      // One decimal place, including negative temperatures
      int32_t t=temp10;
      bool negative=t<0;
      if (negative) t=-t;
      snprintf(s,sizeof(s),"TEMP: %s%ld.%ld",negative?"-":"",(long)(t/10),(long)(t%10));
      textAt(0,45,s);
      snprintf(s,sizeof(s),"RH: %u.%u%%",humid10/10,humid10%10);
      textAt(0,56,s);
    } else {
      textAt(0,24,"VOC WAITING");
      textAt(0,37,"CHECK PROTOCOL");
    }
  }
  oledFlush();
}

void setup() {
  oledStart();
  pms.begin(9600);
  Serial.begin(9600); // 21VOC receiver; do not use Serial.print here
}
void loop() {
  pollPMS();
  pollVOC();
  static unsigned long lastRefresh=0;
  if (millis()-lastRefresh>=1000) {
    lastRefresh=millis();
    render();
  }
  yield();
}
