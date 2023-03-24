#include <Arduino.h>
#include "Device.h"
#include "Shared.h"

PTDdevice treater = {"", "PTDTreater"};

static const char* TAG = "Main";

extern QueueHandle_t receiveQueue;
extern QueueHandle_t sendQueue;

void processReceiveQueue(void* parameters) {
    StaticJsonDocument<jsonDocumentSize> doc;
    while (1) {
        if (xQueueReceive(receiveQueue, (void *)&doc, 0) == pdTRUE) {
            if (doc["action"] == "treat") {
                ESP_LOGI(TAG, "Cookie for my dog :-)");
                doc["report"] = "success"; // cookie was delivered
                sendToController(doc);
                continue;
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    startDevice(&treater);
}

static StaticJsonDocument<jsonDocumentSize> report;
bool sniffing = false;

void loop() {
    delay(3000);
    report["action"] = "data";
    report["measurement"] = sniffing ? "Sniffing" : "Not sniffing";
    sendToController(report);
    sniffing = !sniffing;
}