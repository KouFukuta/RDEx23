//M5Core2は送信側

#include <Arduino.h>
#include <M5Core2.h>
#include <AXP192.h>//バイブレーション使ってみる
#include <Ethernet.h>
#include <ArduinoOSCWiFi.h>

AXP192 power;


//Wi-Fi用の設定を以下に書きます　passだけ設定してください
const char *ssid = "M5_Send";         // SSID
const char *pass = "M5rssiii"; // passwordは自由に設定してください
const int incomingPort = 7070;       // 受信ポート番号
const int outgoingPort = 8080;       // 送信ポート番号

const IPAddress ipServer(192, 168, 1, 1);   // server IPアドレス
const IPAddress ipGateway(192, 168, 1, 1);  // gateway IPアドレス
const IPAddress subnet(255, 255, 255, 0);   // サブネットマスク
const IPAddress ipClient(192, 168, 1, 255); // client IPアドレス

const char *host = "192.168.1.255";


//RSSIを受信した時に起動する関数です。
void rcv_rssi(const OscMessage &msg)
{
  int rssi = msg.arg<int>(0);

  M5.Lcd.setCursor(0, 195);
  M5.Lcd.printf("rssi: %d", rssi); //受信したRSSI値を画面に表示します


}

void setup()
{
  M5.begin();
  M5.Lcd.clearDisplay();
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);

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

  OscWiFi.subscribe(incomingPort, "/rssi", rcv_rssi); //この記述で、runからRSSI値が送られてきた時にrcv_rssi関数が起動するようになります。
}


void loop()
{
  OscWiFi.parse(); //何もない場合はrunからの通信を待ち続けます。
}











/*
参考サイト
https://www.remma.net/?p=367

ESP32 Bluetooth　
MACアドレス : 9C:9C:1F:D0:34:7A
*/

/* #include <Arduino.h>
#include <M5Core2.h>
#include "BluetoothSerial.h"

#include <NimBLEDevice.h>

BluetoothSerial SerialBT;

String MACadd = "9C:9C:1F:D0:34:7A";
uint8_t address[6]  = {0x9C, 0x9C, 0x1F, 0xD0, 0x34, 0x7A};
bool connected;

void setup() {
  M5.begin();
  Serial.begin(115200);
  SerialBT.begin("M5Stack_sendTest", true); 
  Serial.println("The device started in master mode, make sure remote BT device is on!");
  
  
  connected = SerialBT.connect(address);
  
  if(connected) {
    Serial.println("Connected Succesfully!");
  } else {
    while(!SerialBT.connected(10000)) {
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app."); 
    }
  }
  // disconnect() may take upto 10 secs max
  if (SerialBT.disconnect()) {
    Serial.println("Disconnected Succesfully!");
  }
  // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
  SerialBT.connect();

}

void loop() {
    SerialBT.write('T');
}
 */