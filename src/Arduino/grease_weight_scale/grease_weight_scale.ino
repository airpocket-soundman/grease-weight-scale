/*
岡山工場缶グリース測定装置　Arduino IDE版
M5Stack Basic
HX711
1kgロードセル

Ver 3.0.0
*/

#define LGFX_M5STACK

#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>  // クラス"LGFX"を準備します
static LGFX lcd;                 // LGFXのインスタンスを作成。
static LGFX_Sprite sprite(&lcd); // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。

uint32_t red   = 0xFF0000;
uint32_t blue  = 0x0000FF;
unit32_t white = 0xFFFFFF;
uint32_t black = 0x000000;
 

void setup() {
  lcd.init();
  lcd.setRotation(0);
  lcd.setBrightness(128);
  lcd.setColorDepth(24);  // RGB888の24ビットに設定

}

void loop() {
  // put your main code here, to run repeatedly:

}
