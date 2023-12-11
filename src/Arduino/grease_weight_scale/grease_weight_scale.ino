/*
岡山工場缶グリース測定装置　Arduino IDE版
M5Stack Basic
HX711
1kgロードセル

Ver 3.0.0
*/

#define DEBUG

String        version             = "3.0.0";
String        device_number       = "S03";

String        Title_str           = "SCALE " + version;

// 重量計算用定数
double A = 0.0008951;
double B = 34301;

#include <M5Stack.h>
#include "HX711.h"
#define LOADCELL_DOUT_PIN 36
#define LOADCELL_SCK_PIN  26


#define LGFX_M5STACK

#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>  // クラス"LGFX"を準備します
static LGFX lcd;                 // LGFXのインスタンスを作成。
static LGFX_Sprite sprite(&lcd); // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。

uint32_t red   = 0xFF0000;
uint32_t blue  = 0x0000FF;
uint32_t white = 0xFFFFFF;
uint32_t black = 0x000000;

int title_rect[4]     = {  0,  0, 320, 20};   //{x, y, w, h}
int status_circle[4]  = {180, 60,  60};       //{x, y, r,} 



void setup() {
  m5.begin();
  m5.Power.begin();
  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(128);
  lcd.setColorDepth(24);  // RGB888の24ビットに設定

  lcd.fillScreen(TFT_BLACK);
  lcd.drawRect(title_rect[0], title_rect[1], title_rect[2], title_rect[3], TFT_BLUE);
  lcd.setTextColor(TFT_WHITE, TFT_BLUE);
  lcd.setFont(&fonts::Font0);
  lcd.setCursor(0,  0);
  lcd.print(Title_str);
  lcd.setCursor(0,160);
  lcd.print(device_number);
  lcd.setCursor(0,240);
  lcd.print(version);
  lcd.fillCircle(status_circle[0], status_circle[1], status_circle[2], TFT_BLACK);
  lcd.drawCircle(status_circle[0], status_circle[1], status_circle[2], TFT_WHITE);
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
