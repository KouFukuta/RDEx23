#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup(){
  SerialBT.begin("kou'sESP");
}

void loop(){
  SerialBT.print("Send message...");
  delay(1000);
}