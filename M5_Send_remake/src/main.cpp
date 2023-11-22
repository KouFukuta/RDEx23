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
int strongRssi = 10000;
int digitalMic;
double freqPeak;

void rcv_data(const OscMessage &msg)
{
  int rssi;
  int dig_res;
  double peak;
  OscWiFi.subscribe(incomingPort, "/data", rssi, dig_res, peak);

  if(rssi < strongRssi && rssi > 0)
  {
    strongRssi = rssi;
    digitalMic = dig_res;
    freqPeak = peak;
  }
}

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
}

void loop()
{
  //OSC通信でデータを受け取る、rcv_data起動
  OscWiFi.subscribe(incomingPort, "/data", rcv_data);

  M5.Lcd.setCursor(0, 155);
  M5.Lcd.printf("peak: %2lf", freqPeak);
  Serial.printf("peak: %2lf\n", freqPeak);

  Serial.printf("digitalMic: %d\n", digitalMic);

  M5.Lcd.setCursor(0, 195);
  M5.Lcd.printf("rssi: %d", strongRssi);
  Serial.printf("rssi: %d\n", strongRssi);


  //ここからNeoPixel関係
  int r, g, b;
  int lightPower = map(strongRssi, 30, 50, 30, 0);
  pixels.setBrightness(lightPower);
  
  if (digitalMic == 1)
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

  delay(200);
}

