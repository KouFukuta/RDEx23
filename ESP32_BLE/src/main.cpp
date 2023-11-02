#include <Esp.h>

#include <NimBLEDevice.h>
#include <Ethernet.h>
#include <ArduinoOSCWiFi.h>

#define LED_BUILTIN 2

// Wi-Fiの設定です passだけ設定してください
const char *ssid = "M5_Send";
const char *pwd = "M5rssiii"; //送信側で設定したものと同じものを使用
const IPAddress ip(192, 168, 1, 201);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

// for ArduinoOSC
const char *host = "192.168.1.255"; //念の為同じネットワーク内の全員に送信します
const int incomingPort = 8080; // 受信ポート番号
const int outgoingPort = 7070; // 送信ポート番号

void setup(){
  Serial.begin(115200);

#ifdef ESP_PLATFORM
  WiFi.disconnect(true, true);
  delay(1000);
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, pwd);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

void loop()
{
  
  int rssi = 0;
  for (int i = 0; i < 100; i++)
  {
    rssi += WiFi.RSSI();
  }
  rssi = rssi / -100;
  Serial.printf("rssi: %d", rssi);
  OscWiFi.send(host, outgoingPort, "/rssi", rssi); //何もない時はRSSI値を送り続けます
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  OscWiFi.post();
  
  
  delay(200);

}


/* 
前のやつ

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_recieveTest"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

}

void loop() {
  if (SerialBT.available()) {

    if (SerialBT.read() == 'T') {
        Serial.print("受け取れた");
    }

    else{
      Serial.print("失敗");
    }
  }
  delay(20);
  digitalWrite(14, LOW);
}
 */

/*
MACアドレス調べたいときのやつ 
#include <Esp.h>

void setup(void) {
  Serial.begin(115200);
  Serial.println("-----------------");
  uint8_t macBT[6];
  esp_read_mac(macBT, ESP_MAC_BT);
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\r\n", macBT[0], macBT[1], macBT[2], macBT[3], macBT[4], macBT[5]);
}

void loop() {
  delay(1000);
}
 */