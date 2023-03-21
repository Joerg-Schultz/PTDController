#include <Arduino.h>
#include <BluetoothSerial.h>
#include "Controller.h"
#include "Shared.h"

PTDdevice controller = {"", "PTDController"};

BluetoothSerial SerialBT;

void setup() {
    Serial.begin(115200);
    SerialBT.begin(controller.type);

    startController();
}

void loop() {
// write your code here
}