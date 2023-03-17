#include <Arduino.h>
#include <WiFi.h>
#include "ClientReceiver.h"

ClientReceiver* feeder;

void setup() {
    Serial.begin(115200);
    feeder = new ClientReceiver("PTDTreater");

    String macAddress = feeder->start();
    Serial.print("STA MAC: ");
    Serial.println(macAddress);
}

void loop() {
// write your code here
}