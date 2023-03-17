#include <Arduino.h>
#include "BluetoothSerial.h"
#include <WiFi.h>

BluetoothSerial SerialBT;

void setup() {
    Serial.begin(115200);
    SerialBT.begin("PTDController");
    WiFi.mode(WIFI_AP);
}

void loop() {
// write your code here
}