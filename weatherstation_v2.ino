#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <GyverBME280.h>
#include <microDS3231.h>

#define TFT_CS   10
#define TFT_RST  9
#define TFT_DC   8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
GyverBME280 bme;
MicroDS3231 rtc; 

int displayState = 0;
unsigned long lastUpdate = 0;
unsigned long lastSend = 0;
int lightPin = A2;

String utf8rus(String source) {
  int i, k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;
  while (i < k) {
    n = source[i]; i++;
    if (n >= 0xBF) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x2F;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB7; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x6F;
          break;
        }
      }
    }
    m[0] = n; target += String(m);
  }
  return target;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  tft.fillScreen(ST7735_BLACK);


  if (!bme.begin(0x76)) {
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(1);
    tft.setCursor(0, 30);
    tft.println(utf8rus("Ошибка BME280!"));
    while (1);
  }

  Wire.beginTransmission(0x68);
  bool rtcFound = (Wire.endTransmission() == 0);

  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(0, 10);
  if (rtcFound) {
    tft.println(utf8rus("DS3231: OK"));
  } else {
    tft.println(utf8rus("DS3231: не найден"));
  }

  delay(2000); 
}

void loop() {
  if (millis() - lastUpdate > 3000) {
    lastUpdate = millis();
    displayData();
  }

  if (millis() - lastSend > 2000) {
    lastSend = millis();

    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float press = bme.readPressure() / 100.0;
    int light_value = analogRead(lightPin);
    float lightPerc = constrain(light_value * 0.0976, 0, 100);

    Serial.print(temp, 1);
    Serial.print(",");
    Serial.print(hum, 1);
    Serial.print(",");
    Serial.print(press, 1);
    Serial.print(",");
    Serial.println(lightPerc, 1);
  }
}

void displayData() {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);

  static bool rtcOK = false;
  static unsigned long lastRTCCheck = 0;
  if (millis() - lastRTCCheck > 10000) {
    Wire.beginTransmission(0x68);
    rtcOK = (Wire.endTransmission() == 0);
    lastRTCCheck = millis();
  }

  switch (displayState) {
    case 0:
      tft.setCursor(10, 40);
      tft.print(utf8rus("Время: "));
      tft.setTextSize(3);
      tft.setCursor(10, 70);
      tft.setTextColor(rtcOK ? ST7735_BLUE : ST7735_RED);
      if (rtcOK) {
        tft.print(rtc.getTimeString());
      } else {
        tft.print("--:--:--");
      }
      break;

    case 1:
      tft.setCursor(10, 40);
      tft.print(utf8rus("Температура:"));
      tft.setTextSize(3);
      tft.setCursor(10, 70);
      tft.setTextColor(ST7735_YELLOW);
      tft.print(bme.readTemperature(), 1);
      tft.print(" C");
      break;

    case 2:
      tft.setCursor(10, 40);
      tft.print(utf8rus("Давление: "));
      tft.setTextSize(2);
      tft.setCursor(10, 70);
      tft.setTextColor(ST7735_GREEN);
      tft.print(bme.readPressure() / 100, 1);
      tft.print(" hPa");
      break;

    case 3:
      tft.setCursor(10, 40);
      tft.print(utf8rus("Влажность: "));
      tft.setTextSize(3);
      tft.setCursor(10, 70);
      tft.setTextColor(ST7735_CYAN);
      tft.print(bme.readHumidity(), 1);
      tft.print("%");
      break;

    case 4:
      {
        int light_value = analogRead(lightPin);
        float lightPerc = constrain(light_value * 0.0976, 0, 100);
        tft.setCursor(10, 40);
        tft.print(utf8rus("Освещённость:"));
        tft.setTextSize(3);
        tft.setCursor(10, 70);
        tft.setTextColor(ST7735_RED);
        tft.print(lightPerc, 1);
        tft.print("%");
      }
      break;
  }

  displayState = (displayState + 1) % 5;
}
