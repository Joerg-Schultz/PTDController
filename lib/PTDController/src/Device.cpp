//
// Created by Joerg on 22.03.2023.
//

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#include <vector>
#include "Device.h"
#include "Shared.h"

static const int msg_queue_len = 5;
QueueHandle_t receiveQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t sendQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));


static const char* TAG = "Device";

static std::vector<esp_now_peer_info_t> controllerList = {};


static void processSendQueue(void* parameters) {
    StaticJsonDocument<jsonDocumentSize> doc;
    while (1) {
        if (xQueueReceive(sendQueue, (void *) &doc, 0) == pdTRUE) {
            clientMessage myMessage = {};
            serializeJson(doc,myMessage.content);
            for(esp_now_peer_info_t controller : controllerList) {
                String controllerMac = mac2String(controller.peer_addr);
                ESP_LOGI(TAG, "Sending %s to %s", myMessage.content, controllerMac.c_str());
                esp_err_t result = esp_now_send(controller.peer_addr, (uint8_t *) &myMessage, sizeof(myMessage) + 2);  //Sending "jsondata"
                ESP_LOGI(TAG, "Sending: %s", (result == ESP_OK) ? "Success" : "Failed");
            }
        }
    }
}

static void processReceiveQueue(void* parameters) {
    while (1) {

    }
}


static bool sendToController(char jsonString[jsonDocumentSize]) {
    clientMessage myMessage = {};
    memcpy(&myMessage.content, jsonString, sizeof(myMessage.content));
    for (esp_now_peer_info_t controller : controllerList) {
        esp_err_t result = esp_now_send(controller.peer_addr, (uint8_t *) &myMessage, sizeof(myMessage) + 2);  //Sending "jsondata"
        ESP_LOGI(TAG, "Send content: %s", myMessage.content);
        if (result != ESP_OK) return false;
    }
    return true;
}

static StaticJsonDocument<jsonDocumentSize> doc; //TODO having this here is ugly. can I get it into function? Without nulling it when leaving function??
static bool introduceToController(PTDdevice* device) {
    doc["action"] = "handshake"; // TODO use Enum type.
    doc["type"] = device->type;

    if( xQueueSend(sendQueue, (void *)&doc, 10) == pdTRUE) {
        ESP_LOGI(TAG, "submitted handshake to queue");
    } else {
        ESP_LOGI(TAG, "failed to submit handshake to queue");
    }
    return true;
}

bool startDevice(PTDdevice* device) {
    WiFi.mode(WIFI_STA);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    if (esp_now_init() == ESP_OK) {
        ESP_LOGI(TAG,"ESPNow Init Success");
    } else {
        ESP_LOGI(TAG,"ESPNow Init Failed");
        return false;
    }

    int16_t scanResults = WiFi.scanNetworks();
    if (scanResults == 0) {
        ESP_LOGI(TAG, "No WiFi devices in AP Mode found");
        return false;
    }
    ESP_LOGI(TAG,"Found %d devices", scanResults);

   /* xTaskCreate(processReceiveQueue,
                "Reading",
                2048,
                NULL,
                0,
                NULL); */
    BaseType_t createSend = xTaskCreate(processSendQueue,
                                        "Sending",
                                        4096,
                                        NULL,
                                        0,
                                        NULL);
    if(createSend == pdPASS) {
        ESP_LOGI(TAG, "send Task created");
    } else {
        ESP_LOGI(TAG, "send Task failed");
    }

    for (int i = 0; i < scanResults; ++i) {
        String SSID = WiFi.SSID(i);
        String BSSIDstr = WiFi.BSSIDstr(i);
        if (SSID.indexOf("PTDController") == 0) {
            ESP_LOGI(TAG, "Device %d name is %s", i, SSID.c_str());
            esp_now_peer_info_t controller = {};
            controller.channel = CHANNEL;
            controller.encrypt = ENCRYPT;
            int mac[6];
            if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4],
                            &mac[5])) {
                for (int ii = 0; ii < 6; ++ii) {
                    controller.peer_addr[ii] = (uint8_t) mac[ii];
                }
            }
            esp_err_t addStatus = esp_now_add_peer(&controller);
            if (addStatus == ESP_OK) {
                ESP_LOGI(TAG, "Pair success");
                controllerList.push_back(controller);
                introduceToController(device);
            } else {
                ESP_LOGI(TAG, "Pair fail");
                return false;
            }
        }
    }

    return true;
}