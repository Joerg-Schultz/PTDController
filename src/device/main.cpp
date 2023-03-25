#include <Arduino.h>
#include "Device.h"
#include "Shared.h"

PTDdevice treater = {"", "PTDTreater"};

static const char* TAG = "Main";

extern QueueHandle_t receiveQueue;
extern QueueHandle_t sendQueue;

void processReceiveQueue(void *parameters) {
    while (1) {
        if (uxQueueMessagesWaiting(receiveQueue) > 0) {

            StaticJsonDocument<jsonDocumentSize> *doc;
            if (xQueueReceive(receiveQueue, &doc, 0) == pdTRUE) {
                ESP_LOGI(TAG, "Got action!");
                if ((*doc)["action"] == "treat") {
                    ESP_LOGI(TAG, "Cookie for my dog :-)");
                    (*doc)["report"] = "success"; // cookie was delivered
                    sendToController(doc);
                    continue;
                }
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    startDevice(&treater);
}

bool sniffing = false;

void loop() {
    delay(3000);
    auto* report = new StaticJsonDocument<jsonDocumentSize>();
    (*report)["action"] = "data";
    (*report)["measurement"] = sniffing ? "Sniffing" : "Not sniffing";
    sendToController(report);
    sniffing = !sniffing;
}