/* 
手持ちデバイスを親機にする試み
Wi-Fiを繋ぎなおすタイムラグが減るのではないか
オレンジ色のほうに書き込む
*/

#include <Arduino.h>
#include <M5Core2.h>
#include <Ethernet.h>
#include <ArduinoOSCWiFi.h>
#include <Adafruit_NeoPixel.h>

//NeoPixel関連
#define PIN 32
#define NUMPIXELS 24
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//ここからWi-Fiの設定
const char *ssid = "M5_Send_Temochi";
const char *pass = "sendSide_Temochi";
const int incomingPort = 7070;       // 受信ポート番号
const int outgoingPort = 8080;       // 送信ポート番号

const IPAddress ipServer(192, 168, 1, 1);   // server IPアドレス
const IPAddress ipGateway(192, 168, 1, 1);  // gateway IPアドレス
const IPAddress subnet(255, 255, 255, 0);   // サブネットマスク
const IPAddress ipClient(192, 168, 1, 255); // client IPアドレス
const char *host = "192.168.1.255";


//RSSIの値をOSCで受け取った時
int rssi1 = 0;
int rssi2 = 0;
//int rssi3 = 0;
//int rssi4 = 0;

int i = 0;

void rcv_rssi1(const OscMessage &msg)
{
    rssi1 = msg.arg<int>(0);
}

void rcv_rssi2(const OscMessage &msg)
{
    rssi2 = msg.arg<int>(0);
}
/* 
void rcv_rssi3(const OscMessage &msg)
{
  rssi3 = msg.arg<int>(0);
}

void rcv_rssi4(const OscMessage &msg)
{
  rssi4 = msg.arg<int>(0);
}
 */

//マイクのデジタル値をOSCで受け取った時
int dig_res1 = 0;
int dig_res2 = 0;
//int dig_res3 = 0;
//int dig_res4 = 0;

void rcv_digRes1(const OscMessage &msg)
{
  dig_res1 = msg.arg<int>(0);
}

void rcv_digRes2(const OscMessage &msg)
{
  dig_res2 = msg.arg<int>(0);
}
/* 
void rcv_digRes3(const OscMessage &msg)
{
  dig_res3 = msg.arg<int>(0);
}

void rcv_digRes4(const OscMessage &msg)
{
  dig_res4 = msg.arg<int>(0);
}
*/

//FFTのピーク値をOSCで受け取った時
double peak1 = 0;
double peak2 = 0;
//double peak3 = 0;
//double peak4 = 0;

void rcv_peak1(const OscMessage &msg)
{
  peak1 = msg.arg<double>(0);
}

void rcv_peak2(const OscMessage &msg)
{
  peak2 = msg.arg<double>(0);
}
/* 
void rcv_peak3(const OscMessage &msg)
{
  peak3 = msg.arg<double>(0);
}

void rcv_peak4(const OscMessage &msg)
{
  peak4 = msg.arg<double>(0);
}
 */

void setup()
{
  M5.begin();
  Serial.begin(115200);
  M5.Lcd.clearDisplay();
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);

  pixels.begin();
  pixels.setBrightness(0);

  WiFi.softAP(ssid, pass);
  delay(100);
  WiFi.softAPConfig(ipServer, ipGateway, subnet);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("ssid: %s", ssid);
  M5.Lcd.setCursor(0, 65);
  M5.Lcd.printf("incomingPort: %d", incomingPort);
  M5.Lcd.setCursor(0, 105);
  M5.Lcd.printf("outgoingPort: %d", outgoingPort);
  delay(1000);

  //OSC通信でRSSIを受け取る、rcv_rssi起動
  OscWiFi.subscribe(incomingPort, "/rssi1", rcv_rssi1);
  OscWiFi.subscribe(incomingPort, "/rssi2", rcv_rssi2);
  //OscWiFi.subscribe(incomingPort, "/rssi3", rcv_rssi3);
  //OscWiFi.subscribe(incomingPort, "/rssi4", rcv_rssi4);

  OscWiFi.subscribe(incomingPort, "/dig_res1", rcv_digRes1);
  OscWiFi.subscribe(incomingPort, "/dig_res2", rcv_digRes2);
  //OscWiFi.subscribe(incomingPort, "/dig_res3", rcv_digRes3);
  //OscWiFi.subscribe(incomingPort, "/dig_res4", rcv_digRes4);

  OscWiFi.subscribe(incomingPort, "/peak1", rcv_peak1);
  OscWiFi.subscribe(incomingPort, "/peak2", rcv_peak2);
  //OscWiFi.subscribe(incomingPort, "/peak3", rcv_peak3);
  //OscWiFi.subscribe(incomingPort, "/peak4", rcv_peak4);
}

void loop()
{
  //RSSIの強いところの値を取りたい
  int strongRssi = 0;
  int digMic = 0;
  double freqPeak = 0;

  if(rssi1 < rssi2)
  {
    strongRssi = rssi1;
    digMic = dig_res1;
    freqPeak = peak1;
  }
  else
  {
    strongRssi = rssi2;
    digMic = dig_res2;
    freqPeak = peak2;
  }

  M5.Lcd.setCursor(0, 155);
  M5.Lcd.printf("peak: %2lf", freqPeak);
  Serial.printf("peak: %2lf\n", freqPeak);

  Serial.printf("digitalMic: %d\n", digMic);

  M5.Lcd.setCursor(0, 195);
  M5.Lcd.printf("rssi: %d", strongRssi);
  Serial.printf("peak: %d\n", strongRssi);


  //ここからNeoPixel関係
  int r, g, b;
  int lightPower = map(strongRssi, 30, 50, 30, 0);
  pixels.setBrightness(lightPower);
  
  if (digMic == 1)
  {
    for (int j = 0; j < NUMPIXELS; j++)
    {
      if (freqPeak <= 300)
      {
        b = 255;
        g = map(freqPeak, 0, 300, 0, 255);
      }
      else if (freqPeak > 301 && freqPeak <= 600)
      {
        g = 255;
        b = map(freqPeak, 301, 700, 255, 0);
      }
      else if (freqPeak > 601 && freqPeak <= 900)
      {
        g = 255;
        r = map(freqPeak, 601, 900, 0, 255);
      }
      else if (freqPeak > 901 && freqPeak <= 1200)
      {
        r = 255;
        g = map(freqPeak, 901, 1200, 255, 0);
      }
      pixels.setPixelColor(j, pixels.Color(r, g, b));
    }
  }
  else
  {
    pixels.setBrightness(0);
  }

  pixels.show();
  OscWiFi.parse();//Recieve側からの通信を待つ

  delay(50);

}

