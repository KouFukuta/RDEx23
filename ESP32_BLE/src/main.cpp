#include <Esp.h>
#include "BluetoothSerial.h"

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