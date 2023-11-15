//受信側、Wi-Fiを受信する側
//FFTのPeakを受信する、RSSIを送信する
//オレンジのほうに書き込む

//いずれはWiFiMultiに対応させて複数アクセスポイントにアクセスできるようにする
//rssiがいちばん近いM5orESPに接続して、そこで通信をするようにする。

#include <Arduino.h>
#include <M5Core2.h>
#include <ArduinoOSCWiFi.h>
#include <WiFi.h>
#include <WiFiMulti.h> //いずれ使いたいWiFiMulti
#include <Adafruit_NeoPixel.h>

//NeoPixelの設定
#define PIN 32
#define NUMPIXELS 24

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


//ここからWiFiMultiの実装(未完成)
WiFiMulti wifiMulti;

//2チャンネル分を登録(仮)
//いずれは5チャンネル分

/* void wifiConnect()
{

  wifiMulti.addAP("M5_Send01", "sendSide01");
  wifiMulti.addAP("M5_Send02", "sendSide02");

  while(WiFi.status() != WL_CONNECTED)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Wi-Fi Search...");
    wifiMulti.run();
    delay(100);

    if(WiFi.status() == WL_CONNECTED)
    {
      M5.Lcd.fillScreen(BLACK);
      delay(1000);
    }

    else
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.printf("Wi-Fi Nothing!!");
      WiFi.disconnect();
      wifiMulti.run();
      delay(100);
    }
  }
}
 */

// Wi-Fiの設定
/* 
const char *ssid = "M5_Send01";
const char *pwd = "sendSide01"; 
const IPAddress ip(192, 168, 1, 201);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
*/


// for ArduinoOSC
const char *host = "192.168.1.255"; //念の為同じネットワーク内の全員に送信します

const int incomingPort = 8080; // 受信ポート番号
const int outgoingPort = 7070; // 送信ポート番号


//ここからピーク値を取得するところ
double peak = 0;
int dig_res = 0;

void rcv_peak(const OscMessage &msg)//ピーク値を受信したとき
{ 
  peak = msg.arg<double>(0);
  
  
  M5.lcd.setCursor(0, 155);
  M5.Lcd.printf("Peak: %2lf", peak);
}

void rcv_dig_res(const OscMessage &msg)
{
  dig_res = msg.arg<int>(0);
  Serial.printf("%d\n", dig_res);


}



void setup()
{
  M5.begin();
  WiFi.begin();
  M5.IMU.Init();


  // NeoPixelの初期化
  pixels.begin();
  pixels.setBrightness(0);

  //wifiConnect();
  wifiMulti.addAP("M5_Send01", "sendSide01");
  wifiMulti.addAP("M5_Send02", "sendSide02");


  M5.Lcd.clearDisplay();
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);

/* #ifdef ESP_PLATFORM
  WiFi.disconnect(true, true);
  delay(1000);
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, pwd);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }  */
  
/*   M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("ssid: ");
  M5.Lcd.print(WiFi.SSID()); */

  M5.Lcd.setCursor(0, 65);
  M5.Lcd.printf("incomingPort: %d", incomingPort);
  M5.Lcd.setCursor(0, 105);
  M5.Lcd.printf("outgoingPort: %d", outgoingPort);
  delay(1000);

  OscWiFi.subscribe(incomingPort, "/peak", rcv_peak);
  OscWiFi.subscribe(incomingPort, "/dig_res", rcv_dig_res);

}

void loop()
{

  if(WiFi.status() != WL_CONNECTED || WiFi.status() == WL_DISCONNECTED)
  {
    WiFi.disconnect();
    wifiMulti.run();
    //delay(1000);
  }
  
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("ssid: ");
  M5.Lcd.print(WiFi.SSID());

  
  int rssi = 0;
  for (int i = 0; i < 100; i++)
  {
    rssi += WiFi.RSSI();
  }
  rssi = rssi / -100;
  M5.Lcd.setCursor(0, 195);
  M5.Lcd.printf("rssi: %d", rssi);

  OscWiFi.send(host, outgoingPort, "/rssi", rssi); //何もない時はRSSI値を送り続けます
  OscWiFi.post();

  OscWiFi.parse();//sendの通信を待つ


  //ここからNeoPixel関係
  int r, g, b;
  int lightPower = map(rssi, 20, 60, 30, 0);
  pixels.setBrightness(lightPower);

  // ピーク値によってグラデーションしたい
  //今は全部光らせるような設定
  //マイクのデジタル取得値が1になったときに実行
  if (dig_res == 1)
  {
    for (int j = 0; j < NUMPIXELS; j++)
    {

      if (peak <= 300)
      {
        b = 255;
        g = map(peak, 0, 300, 0, 255);
      }

      else if (peak > 301 && peak <= 600)
      {
        g = 255;
        b = map(peak, 301, 700, 255, 0);
      }

      else if (peak > 601 && peak <= 900)
      {
        g = 255;
        r = map(peak, 601, 900, 0, 255);
      }

      else if (peak > 901 && peak <= 1200)
      {
        r = 255;
        g = map(peak, 901, 1200, 255, 0);
      }

      pixels.setPixelColor(j, pixels.Color(r, g, b));
    }
  }

  else
  {
    pixels.setBrightness(0);
  }

  pixels.show();

  //delay(50);

}