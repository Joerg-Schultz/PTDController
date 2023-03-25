#include <Arduino.h>
#include <BluetoothSerial.h>
#include <vector>
#include "Controller.h"
#include "Shared.h"

static const char* TAG = "Main";
extern QueueHandle_t deviceReceiveQueue;
extern QueueHandle_t pairingQueue;
extern std::vector<PTDdevice> deviceList;


PTDdevice controller = {"", "PTDController"};

BluetoothSerial SerialBT;

void processDeviceReceiveQueue(void *parameters) {
    while (1) {
        if (uxQueueMessagesWaiting(deviceReceiveQueue) > 0) {
            StaticJsonDocument<jsonDocumentSize> *received;

            if (xQueueReceive(deviceReceiveQueue, &received, 0) == pdTRUE) {
                if ((*received)["action"] == "handshake") {
                    static StaticJsonDocument<jsonDocumentSize> pairWithClient;
                    String sender = (*received)["sender"];
                    String type = (*received)["type"];
                    pairWithClient["sender"] = sender;
                    pairWithClient["type"] = type;
                    ESP_LOGI(TAG, "shaking with mac %s of type %s", sender.c_str(), type.c_str());
                    xQueueSend(pairingQueue, (void *) &pairWithClient, 10);
                    delete received;
                    continue;
                }
                if ((*received)["action"] == "data") {
                    String value = (*received)["measurement"];
                    ESP_LOGI(TAG, "Got Measurement: %s", value.c_str());
                    delete received;
                    continue;
                }
                if ((*received)["action"] == "treat") {
                    ESP_LOGI(TAG, "Treating was a %s", ((*received)["report"] == "success") ? "Success" : "Fail");
                    delete received;
                    continue;
                }
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
    delay(3000);
    static StaticJsonDocument<jsonDocumentSize> action;
    if (!deviceList.empty()) {
        action["action"] = "treat";
        //ESP_LOGI(TAG, "Sending treat");
        //sendToDeviceViaType("PTDTreater", &action);
    }
}
