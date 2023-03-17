#include <Arduino.h>
#include <BluetoothSerial.h>
#include "Controller.h"

Controller* controller;

BluetoothSerial SerialBT;
void setup() {
    Serial.begin(115200);
    controller = new Controller();
    SerialBT.begin(controller->getName());

    String macAddress = controller->start();
    Serial.print("AP MAC: ");
    Serial.println(macAddress);
}

void loop() {
// write your code here
}