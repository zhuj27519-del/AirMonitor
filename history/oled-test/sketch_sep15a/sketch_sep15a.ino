#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 屏幕宽128、高64；-1表示没有独立复位引脚
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

const uint32_t durationMs = 60000; // 测试时长：60秒
uint32_t startMs;                 // 记录开始时间
int lastSeconds = -1;             // 上次显示的秒数
bool finished = false;           // 是否已经结束

void setup() {
  Serial.begin(115200);

  // I²C：SDA接D2，SCL接D1
  Wire.begin(D2, D1);

  // 使用已配置的I²C接口，屏幕地址为0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println("OLED buffer allocation failed.");
    while (true) {
      delay(100);
    }
  }

  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextWrap(false);

  startMs = millis(); // 从此刻开始计时
  Serial.println("Countdown started.");
}

void loop() {
  if (finished) {
    delay(10);
    return; // 完成后保留DONE画面
  }

  uint32_t elapsed = millis() - startMs;

  // 向上取整剩余秒数，依次显示60、59……0
  int secondsLeft = 0;
  if (elapsed < durationMs) {
    secondsLeft = (durationMs - elapsed + 999) / 1000;
  }

  // 只有秒数变化时才更新画面
  if (secondsLeft != lastSeconds) {
    lastSeconds = secondsLeft;
    oled.clearDisplay();

    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("POLLEN TIMER TEST");

    oled.setTextSize(3);
    oled.setCursor(0, 24);

    if (secondsLeft > 0) {
      oled.print(secondsLeft);
      oled.print(" s");
    } else {
      oled.print("DONE");
      finished = true;
      Serial.println("Countdown finished.");
    }

    oled.display(); // 将画面真正发送到屏幕
  }

  delay(10);
}