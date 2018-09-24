/*  voltmeter_SD by K.Sakurai 20180714
 *  あちゃんでいいの使用デバイスで電圧を測定してSDカードに記録するデータロガーです。
 *  使用ライブラリは Wire(I2C), SD, Adafruit_GFX, Adafruit_SSD1306
 *  Wireはプログラム中一回も使ってないように見えるけどたぶんAdafruit_SSD1306で使ってるから抜かないで。
 */
 
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <Adafruit_SSD1306.h>

#include <Wire.h>

#include <SD.h>

//=====【Hardware Configration】ハードウェアに応じて以下を変更すること
const int chipSelect = 10;    //SDピンアサイン CSは10
const int analogPin = 0;    //アナログピンの番号 A0つかいます
const float VCC = 3.3;    //Vccは3.3のデバイスを使用
const int MULTI_HIGH = 390;    //倍率器の抵抗Vcc側
const int MULTI_LOW = 100;    //倍率器の抵抗GND側 high/lowがあってればOK

//=====【Software Configration】測定の仕様を変えたければ以下を変更すること
const int NUMDET = 100;   //1つのデータにつき測定回数は100回
const int INTERVAL = 30;  //測定間隔は30ms
const int WAIT_TIME = 5000; //データ取得間隔は5000ms 処理時間は実測200ms程度なので、300ms以下NG

//=====

Adafruit_SSD1306 display(-1);

int number;

void setup()
{                
  pinMode(LED_BUILTIN, OUTPUT);

  //SDカードスロット初期化
  if (!SD.begin(chipSelect)) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5000);
    // don't do anything more:
    return;
  }

  //OLED初期化
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // aitendoで売ってるOLEDデバイスの I2Cアドレスは0x3C
  display.clearDisplay();

  //データの通し番号初期化
  number = 0;
}

void loop()
{
  int i;
  double val;
  double voltage;
  unsigned long time_zero;

  //開始時刻を格納・valの初期化

  time_zero = millis();
  val = 0;

  //A0から取得して電圧を求めるのを規定回数繰り返す valの中身は回数分の測定値の総和になる
  for(i = 0; i < NUMDET; i++)
  {
    voltage = (double) analogRead(analogPin) * VCC / 1024 * (MULTI_HIGH + MULTI_LOW) / MULTI_LOW * 2;
    val += voltage;
    delay(INTERVAL);
  }

  //平均値を導出 valの中身は平均値になる
  val = val / NUMDET;  
  
  //valをOLEDに出力
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("KiDenRyoku");
  display.setTextSize(2);
  display.print(val, 2);
  display.println("V");
  display.display();

  //SDに書き込む文字列つくる
  String dataString = "";
  dataString += String(number);
  dataString += ",";
  dataString += String(val);
  
  //SDカードに書き込む。ダメならLEDが点灯する。
  File dataFile = SD.open("measure.csv", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  } else { 
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
  } 
  
  //全部終わったら1回チカっとして通し番号をインクリメント
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  number++;

  //WAIT_TIMEまで待機
  while(millis() < time_zero + WAIT_TIME)
  {
    ;
  }
}
