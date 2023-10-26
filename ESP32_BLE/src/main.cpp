#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup(){

  SerialBT.begin("Kou'sESP");

}

void loop(){

  SerialBT.print("send message...");
  delay(500);

}