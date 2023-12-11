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
String        Title_str           = "SCALE                     ID : " + device_number + "  Ver : " + version;

// 重量計算用定数
double A = 0.0008951;
double B = 34301;

#include <M5Stack.h>
#include "HX711.h"
#define LOADCELL_DOUT_PIN 36
#define LOADCELL_SCK_PIN  26
#define WEIGHT_LIST_NUM 5
HX711 scale;
double weight_list[WEIGHT_LIST_NUM] = {0.0};      //
double weight_list_ave    = 0.0;                  //
double weight_list_max    = 0.0;                  //
double weight_list_min    = 0.0;                  //
double stable_threshold   = 1.0;                  //
double zero_reset_value   = 0.0;                  //
double weight_stable      = 0.0;                  //
double weight_prev        = 0.0;                  //
bool   flag_stable        = false;                //
double seight_diff        = 0.0;                  //
       

// EEPROMサイズとアドレスの設定
#include <EEPROM.h>
#define EEPROM_MEM_SIZE       100
#define ADDR_THRE_WEIGHT      0x00

// 閾値設定
#define THRESHOLD_WEIGHT_DEFO 3.0
double threshold_weight = 3.0;


#define LGFX_M5STACK

#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>  // クラス"LGFX"を準備します
static LGFX lcd;                 // LGFXのインスタンスを作成。
static LGFX_Sprite sprite(&lcd); // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。

uint16_t title_rect_pos[4]       = {   0,   0, 320, 20};   //{x, y, w, h}
uint16_t status_circle_pos[3]    = { 240, 110,  60};       //{x, y, r,} 
uint16_t title_pos[2]            = {   2,   2};
uint16_t net_weight_pos[2]       = {  15,  35};
uint16_t diff_pos[2]             = {  15,  78};
uint16_t count_label_pos[2]      = {  20, 162};
uint16_t count_value_pos[2]      = { 100, 160};
uint16_t btn_A_label_pos[2]      = {  33, 224};
uint16_t btn_C_label_pos[2]      = { 220, 224};
uint16_t btn_shiftB_label_pos[2] = { 145, 205};
uint16_t btn_shiftC_label_pos[2] = { 240, 205};
uint16_t threshold_value_pos[2]  = { 280, 185};
uint16_t threshold_unit_pos[2]   = { 305, 195};



void setup() {
  m5.begin();
  m5.Power.begin();

  EEPROM.begin(EEPROM_MEM_SIZE);                          //EEPROM開始
  readThreshold();                                        //閾値読み込み

  Serial.begin(115200);                                   //USBシリアル出力開始
  Serial.println("UART connected");
  Serial.print("grease scale version");
  Serial.println(version);

  lcd.init();                                             //lcd開始、設定
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.setColorDepth(24);  // RGB888の24ビットに設定

  //Title 描画
  lcd.fillScreen(TFT_BLACK);
  lcd.fillRect(title_rect_pos[0], title_rect_pos[1], title_rect_pos[2], title_rect_pos[3], TFT_BLUE);
  lcd.setTextColor(TFT_WHITE, TFT_BLUE);
  lcd.setFont(&fonts::Font2);
  lcd.setCursor(title_pos[0], title_pos[1]);
  lcd.print(Title_str);
  lcd.fillCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2], TFT_BLACK);
  lcd.drawCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2], TFT_WHITE);

  //label描画
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setFont(&fonts::Font2);
  lcd.setCursor(count_label_pos[0],       count_label_pos[1]);
  lcd.print("count=");
  lcd.setCursor(btn_A_label_pos[0],       btn_A_label_pos[1]);
  lcd.print("zero reset");
  lcd.setCursor(btn_C_label_pos[0],       btn_C_label_pos[1]);
  lcd.print("count reset");
  lcd.setCursor(btn_shiftB_label_pos[0],  btn_shiftB_label_pos[1]);
  lcd.print("( + )");
  lcd.setCursor(btn_shiftC_label_pos[0],  btn_shiftC_label_pos[1]);
  lcd.print("( - )");
  lcd.setCursor(threshold_value_pos[0],   threshold_value_pos[1]);
  lcd.printf("%.1f",threshold_weight);
  lcd.setCursor(threshold_unit_pos[0],    threshold_unit_pos[1]);
  lcd.print("g");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);       //HX711初期化


}

void loop() {
  read_weight();
  delay(1000);
}

// 重量を測定し、weight_listに追加。weight_listの平均をweight_ave,weight_listの最大をweiht_list_max,最小をweight_list_minにする
void read_weight(){
  double weight_raw = static_cast<double>(scale.read()) * A;
  //Serial.println(weight_raw);
  //eight_raw *= A;
  Serial.print("weight_raw = ");
  Serial.println(weight_raw);
  for (int i = 1; i < WEIGHT_LIST_NUM; ++i){
    weight_list[i - 1] = weight_list[i];
  }
  weight_list[WEIGHT_LIST_NUM - 1] = weight_raw;
  weight_raw = 0.0;
  weight_list_max = weight_list[0];
  weight_list_min = weight_list[0];
  for (int i = 0; i < WEIGHT_LIST_NUM; ++i){
    weight_raw += weight_list[i];
    if (weight_list_max < weight_list[i]) weight_list_max = weight_list[i];
    if (weight_list_min > weight_list[i]) weight_list_min = weight_list[i];
    Serial.print("weight_list[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.print(weight_list[i]);
    Serial.print(" / "); 
  }
  weight_list_ave = weight_raw / static_cast<double>(WEIGHT_LIST_NUM);
  Serial.println("");
  Serial.print("weight_list_ave = ");
  Serial.print(weight_list_ave);
  Serial.print(" / weight_list_min = ");
  Serial.print(weight_list_min);
  Serial.print(" / weight_lsit_max = ");
  Serial.println(weight_list_max);

}


void readThreshold(){                                      //閾値読み込み。EEPROMが空の場合、デフォルト値を記録
  String read_data;
  String write_data;

  read_data = EEPROM.readString(ADDR_THRE_WEIGHT);            //EEPROM読み込み
  Serial.print("read_data = ");
  Serial.println(read_data);
  if (read_data == ""){      
    threshold_weight = THRESHOLD_WEIGHT_DEFO;
    write_data = String(THRESHOLD_WEIGHT_DEFO);
    writeStringToEEPROM(ADDR_THRE_WEIGHT, write_data);
    EEPROM.commit();
  } else {
    threshold_weight = read_data.toDouble();
  }
  Serial.print("threshold_weight = ");
  Serial.print(threshold_weight);
}

void writeStringToEEPROM(int address, String data) {
  // EEPROMにStringを書き込む
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(address + i, data[i]);
  }
  // Null終端文字を書き込む
  EEPROM.write(address + data.length(), '\0');
}


