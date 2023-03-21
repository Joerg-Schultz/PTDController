#include <Arduino.h>
#include <BluetoothSerial.h>
#include "Controller.h"
#include <esp_now.h>
#include "WiFi.h"

Controller* controller;
esp_now_peer_num_t *peer_num;

BluetoothSerial SerialBT;
void setup() {
    Serial.begin(115200);
    controller = new Controller();
    SerialBT.begin(controller->getName());

    String macAddress = controller->start();
    Serial.print("AP MAC: ");
    Serial.println(macAddress);

    // Queue superviser

}

void loop() {
// write your code here
}