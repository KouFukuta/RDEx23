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

//ジャイロ操作でパンニングぶん回したい
#include <BleConnectionStatus.h>
#include <BleMouse.h>


//NeoPixel関連
#define PIN 32
#define NUMPIXELS 24

#define bottomLED 25
#define NUMBOTTOMS 10

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bottoms(NUMBOTTOMS, bottomLED + NEO_KHZ800);

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


//BLEMouseの設定
BleMouse bleMouse("GyloMouse");
signed char mouse_x = 0;
signed char mouse_y = 0;
float mouse_min = 200;

float accX = 0;
float accY = 0;
float accZ = 0;

float gyroX = 0;
float gyroY = 0;
float gyroZ = 0;

 
//受信した各データをここで格納
int rssi[2];
int dig_res[2];
double peak[2];

void rcv_data(const OscMessage &msg)
{
  //ここでは正常
  int Number = msg.arg<int>(0);
  Serial.printf("No.: %d\n", Number);

  rssi[Number] = msg.arg<int>(1);
  Serial.printf("rssi: %d\n", rssi[Number]);

  dig_res[Number] = msg.arg<int>(2);
  Serial.printf("digitalMic: %d\n", dig_res[Number]);

  peak[Number] = msg.arg<double>(3);
  Serial.printf("peak: %lf\n\n", peak[Number]);
}


void move_mouse()
{
  M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  M5.update();

  mouse_x = 0;
  mouse_y = 0;

  if(accX * 1000 > mouse_min)
  {
    mouse_x = -1 * (accX * 1000) / mouse_min;
  }
  if(accX * 1000 < mouse_min * -1)
  {
    mouse_x = -1 * (accX * 1000) / mouse_min;
  }
  if(accY * 1000 > mouse_min)
  {
    mouse_y = 1 * (accY * 1000) / mouse_min;
  }
  if(accY * 1000 < mouse_min * -1)
  {
    mouse_y = 1 * (accY * 1000) / mouse_min;
  }
  bleMouse.move(mouse_x, mouse_y);
}


void setup()
{
  M5.begin();
  Serial.begin(115200);
  M5.Lcd.clearDisplay();
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);

  //NeoPixelの設定
  pixels.begin();
  bottoms.begin();
  pixels.setBrightness(0);

  //BLEMouseの設定
  M5.IMU.Init();
  bleMouse.begin();

  WiFi.softAP(ssid, pass);
  delay(100);
  WiFi.softAPConfig(ipServer, ipGateway, subnet);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print(ssid);

  //OSC通信でデータを受け取る、rcv_data起動
  OscWiFi.subscribe(incomingPort, "/data", rcv_data);

  delay(1000);
}

void loop()
{
  //BLEMouse
  move_mouse();

  //  配列使うとおかしい
  int strongRssi;
  int digitalMic;
  double freqPeak;
  int device = 0;

  int i = 0;

  if(rssi[i] < rssi[i+1])
  {
    strongRssi = rssi[i];
    digitalMic = dig_res[i];
    freqPeak = peak[i];

    device = i;
  }
  else
  {
    strongRssi = rssi[i+1];
    digitalMic = dig_res[i+1];
    freqPeak = peak[i+1];  

    device = i + 1;
  }

  M5.Lcd.setCursor(0, 65);
  M5.Lcd.printf("Device: Device%d", device);
  M5.Lcd.setCursor(0, 105);
  M5.Lcd.printf("DigitalMic: %d", digitalMic);
  M5.Lcd.setCursor(0, 155);
  M5.Lcd.printf("Peak: %lf", freqPeak);
  M5.Lcd.setCursor(0, 195);
  M5.Lcd.printf("RSSI: %d", strongRssi);

  //ここからNeoPixel関係
  int r, g, b;
  int lightPower = map(strongRssi, 30, 50, 30, 0);

  pixels.setBrightness(lightPower);
  bottoms.setBrightness(lightPower);
  
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
      bottoms.setPixelColor(j, bottoms.Color(r, g, b));
    }
  }
  else
  {
    pixels.setBrightness(0);
    bottoms.setBrightness(0);
  }

  pixels.show();
  bottoms.show();

  //データを待つ
  //OscWiFi.parse();
  OscWiFi.update();

}