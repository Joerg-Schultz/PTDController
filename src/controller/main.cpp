#include <Arduino.h>
#include <BluetoothSerial.h>
#include "Controller.h"
#include "Shared.h"

static const char* TAG = "Main";
extern QueueHandle_t deviceReceiveQueue;
extern QueueHandle_t pairingQueue;

PTDdevice controller = {"", "PTDController"};

BluetoothSerial SerialBT;

void readFromDeviceInQueue(void * parameters) {
    StaticJsonDocument<jsonDocumentSize> doc;
    while(1) {
        if (xQueueReceive(deviceReceiveQueue, (void *)&doc, 0) == pdTRUE) {
            if (doc["action"] == "handshake") {
                String bla = doc["type"];
                ESP_LOGI(TAG, "shaking with type %s", bla.c_str());
                xQueueSend(pairingQueue, (void *)&doc, 10);
                continue;
            }
            if (doc["action"] == "whatever") {
                // do something else
                continue;
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin(controller.type);

    startController(&controller);

}

void loop() {
// write your code here
}