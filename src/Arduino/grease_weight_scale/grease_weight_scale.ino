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
String        Title_str           = "SCALE                  ID : " + device_number + "    Ver : " + version;

// 重量計算用定数
double A                          = 0.0000001 * 9412.198;   //8951
double B                          = 669.42;                 //580.47

#include <M5Stack.h>

// loadcellの設定
#include "HX711.h"
#define LOADCELL_DOUT_PIN 36                                  // HX711 DOUT端子接続PIN番号
#define LOADCELL_SCK_PIN  26                                  // HX711 SCK端子接続PIN番号
#define WEIGHT_LIST_VALID_NUM 5                               // 重量判定の有効要素数
#define WEIGHT_LIST_ELIMINATE_NUM_UPPER 2                     // 上側外れ値除外数
#define WEIGHT_LIST_ELIMINATE_NUM_LOWER 2                     // 下側外れ値除外数
#define WEIGHT_LIST_CENTER_NUM 2                              // 中央値の場所
#define WEIGHT_LIST_NUM (WEIGHT_LIST_VALID_NUM + WEIGHT_LIST_ELIMINATE_NUM_UPPER + WEIGHT_LIST_ELIMINATE_NUM_LOWER)

HX711 scale;
double    weight_list[WEIGHT_LIST_NUM] = {0.0};                   // 重量測定結果のリスト
double    filtered_weight_list[WEIGHT_LIST_VALID_NUM] = {0.0};    // 重量測定結果を降順にsortしたリスト
double    weight_list_ave     =  0.0;                             // filtered_weight_listの平均値
double    weight_list_center  =  0.0;                             // filtered_weight_listの中央値
double    weight_list_max     =  0.0;                             // filtered_weight_listの最大値
double    weight_list_min     =  0.0;                             // filtered_weight_listの最小値
double    weight_list_range   =  0.0;                             // weight_list_max - weight_list_min
double    zero_reset_value    =  0.0;                             // tareをした際のweight_list_center
double    weight_prev         =  0.0;                             // 前回の軽量合格時のweight_list_center
double    weight_diff         =  0.0;                             // weight_prevとweight_list_centerの差
double    weight_gross        =  0.0;                             // weight_list_centerとzero_reset_valueの差
double    threshold_stable    =  0.3;                             // 安定を判定するための閾値　許容するweight_list_max,minの差を設定
double    threshold_mesure    = 10.0;                             // 測定皿に計測対象物が乗っているかどうかを判定する閾値
bool      flag_mesure         = true;                             // 判定を開始しても良いかのフラグ
bool      flag_stable         = false;                            // 重量が安定したかどうかのフラグ
bool      flag_zero_reset     = true;                             // zero_reset後初回のみ処理するためのフラグ
uint32_t  counter             =    0;
       
// EEPROMサイズとアドレスの設定
#include <EEPROM.h>
#define EEPROM_MEM_SIZE       100
#define ADDR_THRE_WEIGHT      0x00

// 閾値設定
#define THRESHOLD_WEIGHT_DEFO 3.0                           //グリス塗布量良否判定の初期値　3.0g
double threshold_weight = THRESHOLD_WEIGHT_DEFO;          

// loyanGFXの導入
#define LGFX_M5STACK
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>    // クラス"LGFX"を準備します
static LGFX lcd;                  // LGFXのインスタンスを作成。
static LGFX_Sprite sprite(&lcd);  // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。

// display layout
uint16_t title_rect_pos[4]        = {   0,   0, 320, 20};   //{x, y, w, h}
uint16_t status_circle_pos[3]     = { 265, 128,  52};       //{x, y, r,} 
uint16_t title_pos[2]             = {  10,   2};            //{x, y}
uint16_t gross_weight_pos[2]      = { 120,  35};
uint16_t diff_weight_pos[2]       = {  10,  68};
uint16_t threshold_value_pos[2]   = { 130, 160};
uint16_t count_value_pos[2]       = {  10, 195};
uint16_t btn_A_label_pos[2]       = {  33, 224};
uint16_t btn_B_label_pos[2]       = { 127, 224};
uint16_t btn_C_label_pos[2]       = { 220, 224};
uint16_t btn_shiftB_label_pos[2]  = { 145, 205};
uint16_t btn_shiftC_label_pos[2]  = { 240, 205};

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
  lcd.setCursor(btn_A_label_pos[0],       btn_A_label_pos[1]);
  lcd.print("   shift  ");
  btn_label_show(); 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);       //HX711初期化

  counter_show();
  threshold_show();
}

void loop() {
  String write_data;
  M5.update();
  read_weight();
  /*
  Serial.print("center = ");
  Serial.print(weight_list_center);
  Serial.print(" / range = ");
  Serial.print(weight_list_range);
  Serial.print(" / weight_diff = ");
  Serial.print(weight_diff);
  Serial.print(" / weight_prev = ");
  Serial.print(weight_prev);
  Serial.print(" / flag_mesure = ");
  Serial.print(flag_mesure);
  Serial.print(" / flag_zero_reset = ");
  Serial.println(flag_zero_reset);
  */
  judge_stable();
  weight_gross_show();  
  counter_show();
  

  if (M5.BtnA.wasPressed() || m5.BtnA.wasReleased()) btn_label_show(); 
  if (M5.BtnB.wasPressed() && m5.BtnA.isReleased()) {
    zero_reset_value = weight_list_center;
    flag_zero_reset = true;
    flag_mesure = true;
  }
  if (M5.BtnC.wasPressed() && m5.BtnA.isReleased()) {
    counter = 0;
    counter_show;
  }
  if (M5.BtnB.wasPressed() && m5.BtnA.isPressed()){
    threshold_weight -= 1.0;
    if (threshold_weight < 1.0) threshold_weight = 1.0;
    write_data = String(threshold_weight);
    writeStringToEEPROM(ADDR_THRE_WEIGHT, write_data);
    EEPROM.commit();
    threshold_show();
  }  
  if (M5.BtnC.wasPressed() && m5.BtnA.isPressed()){ 
    threshold_weight += 1.0;
    write_data = String(threshold_weight);
    writeStringToEEPROM(ADDR_THRE_WEIGHT, write_data);
    EEPROM.commit();
    threshold_show();
  }
}

void judge_stable(){

  if (weight_gross >= threshold_mesure){                                // モノが載っているときの処理
    if (weight_list_range <= threshold_stable){           // 重量が安定している場合の処理          
      flag_stable = true;
      if (flag_zero_reset == true){                                         // zero_reset後一度だけの処理weight_prevによらずgreen表示
        flag_zero_reset = false;
        lcd.fillCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2] - 1, TFT_DARKGREEN);
        lcd.drawCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2], TFT_WHITE);
        weight_prev = weight_list_center;
        flag_mesure = false;
      }
      if (flag_mesure == true){                                             // 一度ものを下ろした場合の処理
        if (weight_diff >= threshold_weight){                                 // 重量差が閾値以上の場合
          lcd.fillCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2] - 1, TFT_DARKGREEN);
          lcd.drawCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2], TFT_WHITE);
          weight_prev = weight_list_center;
          weight_diff_show();
          counter += 1;
        } else {                                                              // 重量差が閾値未満の場合　
          lcd.fillCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2] - 1, TFT_RED);
          lcd.drawCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2], TFT_WHITE);
          weight_diff_show();
        }
        flag_mesure = false;
      } 
    } else {                                                              // 重量が安定していない場合の処理
      flag_stable = false;
    }
  } else {                                                              // モノが載っていないときの処理
    flag_mesure = true;
    lcd.fillCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2] - 1, TFT_BLACK);
    lcd.drawCircle(status_circle_pos[0], status_circle_pos[1], status_circle_pos[2], TFT_WHITE);
  }
}

void counter_show(){
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.setFont(&fonts::Font4);
  lcd.setCursor(count_value_pos[0], count_value_pos[1]);
  lcd.printf("count = %d    ",counter);
}

void threshold_show(){
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.setFont(&fonts::Font4);
  lcd.setCursor(threshold_value_pos[0], threshold_value_pos[1]);
  lcd.printf("/ %.1f g   ",threshold_weight);
}

void weight_gross_show(){
  lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  lcd.setFont(&fonts::Font4);
  lcd.setCursor(gross_weight_pos[0], gross_weight_pos[1]);
  //weight_gross = weight_list_center - zero_reset_value;
  if (weight_gross >= 0.0 ){
    lcd.print(" ");
  } else {
    lcd.print("-");
    weight_gross *= -1.0;
  }
  if (100.0 <= weight_gross && weight_gross < 1000.0) {
    lcd.print("    ");
    lcd.setCursor(gross_weight_pos[0] + 14, gross_weight_pos[1]);
  } else if (10.0 <= weight_gross && weight_gross < 100.0) {
    lcd.print("      ");
    lcd.setCursor(gross_weight_pos[0] + 28, gross_weight_pos[1]);
  } else if (0.0 <= weight_gross && weight_gross < 10.0) {
    lcd.print("        ");
    lcd.setCursor(gross_weight_pos[0] + 42, gross_weight_pos[1]);
  }
  /*else if (0.0 == weight_gross) {
    lcd.print("        ");
    //weight_gross += 0.0001;   //表示桁異常対策のおまじない
    lcd.setCursor(gross_weight_pos[0] + 42, gross_weight_pos[1]);
  }*/

  if (weight_gross >= 1000.0) {
    lcd.print("  O.L.   ");
  } else {
    lcd.printf("%.1f    ", weight_gross);
  }
}

void weight_diff_show(){
  if (flag_stable == true){
    lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  } else {
    lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
  }

  lcd.setFont(&fonts::Font8);
  lcd.setCursor(diff_weight_pos[0], diff_weight_pos[1]);
  //weight_diff = weight_prev - weight_list_center;
  if (weight_diff <= 0.0) weight_diff = 0.0;
  
  if (10.0 <= weight_diff && weight_diff < 100.0) {
    lcd.setCursor(diff_weight_pos[0], diff_weight_pos[1]);
  } else if (0.0 < weight_diff && weight_diff < 10.0) {
    lcd.print("  ");
    lcd.setCursor(diff_weight_pos[0] + 55, diff_weight_pos[1]);
  } else if (0.0 == weight_diff) {
    lcd.print("  ");
    weight_diff += 0.0001;   //表示桁異常対策のおまじない
    lcd.setCursor(diff_weight_pos[0] + 55, diff_weight_pos[1]);
  }

  if (weight_diff >= 100.0) {
    lcd.print("----");
  } else {
    lcd.printf("%.1f", weight_diff);
  }
}

void btn_label_show(){
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setFont(&fonts::Font2);
  if (M5.BtnA.isReleased()){
    lcd.setCursor(btn_B_label_pos[0],       btn_A_label_pos[1]);
    lcd.print("zero reset");
    lcd.setCursor(btn_C_label_pos[0],       btn_C_label_pos[1]);
    lcd.print("count reset");
  } else if (M5.BtnA.isPressed()) {
    lcd.setCursor(btn_B_label_pos[0],       btn_A_label_pos[1]);
    lcd.print("   ( - )    ");
    lcd.setCursor(btn_C_label_pos[0],       btn_C_label_pos[1]);
    lcd.print("   ( + )    ");
  }
}

void read_weight(){     // 重量を測定し、weight_listに追加。weight_listの平均をweight_ave,weight_listの最大をweiht_list_max,最小をweight_list_minにする
  double weight_raw = (static_cast<double>(scale.read()) * A) - B;
  for (int i = 0; i < WEIGHT_LIST_NUM - 1; ++i){
    weight_list[i] = weight_list[i + 1];
  }
  weight_list[WEIGHT_LIST_NUM - 1] = weight_raw;
  weight_raw = 0.0;
  weight_list_filter();
}

void weight_list_filter(){
  double sorted_weight_list[WEIGHT_LIST_NUM] = {0.0};
  for (int i = 0; i < WEIGHT_LIST_NUM; i++){
    sorted_weight_list[i] = weight_list[i];
  }
  sorted_weight_list[WEIGHT_LIST_NUM] = weight_list[0];
  for (int i = 0; i < WEIGHT_LIST_NUM - 1; i++){
    for (int j = i + 1; j < WEIGHT_LIST_NUM; j++){
      if (sorted_weight_list[i] > sorted_weight_list[j]){
        double temp = sorted_weight_list[i];
        sorted_weight_list[i] = sorted_weight_list[j];
        sorted_weight_list[j] = temp;
      }
    } 
  }
  weight_list_ave = 0.0;
  for (int i = 0; i < WEIGHT_LIST_VALID_NUM; i++){
    filtered_weight_list[i] = sorted_weight_list[i + WEIGHT_LIST_ELIMINATE_NUM_LOWER];
    weight_list_ave += filtered_weight_list[i];
  }
  weight_list_ave    /= WEIGHT_LIST_VALID_NUM;
  weight_list_center  = filtered_weight_list[WEIGHT_LIST_CENTER_NUM];
  weight_list_min     = filtered_weight_list[0];
  weight_list_max     = filtered_weight_list[WEIGHT_LIST_VALID_NUM - 1];
  weight_gross        = weight_list_center - zero_reset_value;
  weight_diff         = round((weight_prev - weight_list_center) * 10) / 10;
  weight_list_range   = weight_list_max - weight_list_min;
}

void readThreshold(){   //閾値読み込み。EEPROMが空の場合、デフォルト値を記録
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


