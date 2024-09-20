#include <M5Unified.h>
#include <Ethernet.h>
#include <ArduinoOSCWiFi.h>

//ジャイロ操作
#include <BleConnectionStatus.h>
#include <BleMouse.h>

//スピーカー数
#define DEVICE_COUNT 4

//NeoPixel
#include <Adafruit_NeoPixel.h>
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


//BLEMouseの設定
BleMouse bleMouse("StickCMouse");
signed char mouse_x = 0;
signed char mouse_y = 0;
float mouse_min = 200;

float accX = 0;
float accY = 0;
float accZ = 0;

//受信した各データをここで格納
int rssi[DEVICE_COUNT] = {};
int dig_res[DEVICE_COUNT] = {};
double peak[DEVICE_COUNT] = {};

int strongRssi;
int digitalMic;
double freqPeak;
int device = 0;

//子機からデータ受信はここ
void rcv_data(const OscMessage &msg)
{
  device = 0;

  int Number = msg.arg<int>(0);
  Serial.printf("No.: %d\n", Number);

  rssi[Number] = msg.arg<int>(1);
  Serial.printf("rssi: %d\n", rssi[Number]);

  dig_res[Number] = msg.arg<int>(2);
  Serial.printf("digitalMic: %d\n", dig_res[Number]);

  peak[Number] = msg.arg<double>(3);
  Serial.printf("peak: %lf\n\n", peak[Number]);

  for(int i = 0; i < DEVICE_COUNT; i++)
  {
    if(rssi[i] < rssi[device])
    {
      device = i;
    }
  }
  Serial.print(device);
}

//BLEMouseはここ
void move_mouse()
{

  M5.Imu.getAccelData(&accX,&accY,&accZ);

  mouse_x = 0;
  mouse_y = 0;

  if(accX * 1000 > mouse_min)
    mouse_x = -1 * (accX * 2000) / mouse_min;

  if(accX * 1000 < mouse_min * -1)
    mouse_x = -1 * (accX * 2000) / mouse_min;

  if(accY * 1000 > mouse_min)
    mouse_y = 1 * (accY * 2000) / mouse_min;

  if(accY * 1000 < mouse_min * -1)
    mouse_y = 1 * (accY * 2000) / mouse_min;

  bleMouse.move(mouse_x, mouse_y);
  delay(10);

}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);

  M5.Display.setTextSize(2);

  bleMouse.begin();
  
  pixels.begin();
  pixels.setBrightness(0);

  WiFi.softAP(ssid, pass);
  delay(100);
  WiFi.softAPConfig(ipServer, ipGateway, subnet);

  OscWiFi.subscribe(incomingPort, "/data", rcv_data);

  delay(1000);

}

void loop()
{
  OscWiFi.update();
  move_mouse();

  //強度の強いRSSIの子機のデータを代入
  strongRssi = rssi[device];
  digitalMic = dig_res[device];
  freqPeak = peak[device];

  //NeoPixel表示
  int r, g, b;
   
  int lightPower = map(strongRssi, 30, 55, 15, 0);

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
    }
  }
  else
  {
    pixels.setBrightness(0);
  }

  pixels.show();

  delay(10);
}