/*
参考サイト
https://www.remma.net/?p=367

ESP32 Bluetooth　
MACアドレス : 9C:9C:1F:D0:34:7A
*/

#include <Arduino.h>
#include <M5Core2.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

String MACadd = "9C:9C:1F:D0:34:7A";
uint8_t address[6]  = {0x9C, 0x9C, 0x1F, 0xD0, 0x34, 0x7A};
bool connected;

void setup() {
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
