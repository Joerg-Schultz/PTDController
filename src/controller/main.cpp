#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include "Controller.h"

const String name = "PTDController";

BluetoothSerial SerialBT;
void setup() {
    Serial.begin(115200);
    SerialBT.begin(name);
    Controller controller = Controller(name);
    String macAddress = controller.start();
    Serial.print("AP MAC: ");
    Serial.println(macAddress);
}

void loop() {
// write your code here
}