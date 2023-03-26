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
    while (1) {
        if (uxQueueMessagesWaiting(sendQueue) > 0) {
            StaticJsonDocument<jsonDocumentSize> *doc;

            if (xQueueReceive(sendQueue, &doc, 0) == pdTRUE) {
                clientMessage myMessage = {};
                serializeJson((*doc), myMessage.content);
                for (esp_now_peer_info_t controller: controllerList) {
                    String controllerMac = mac2String(controller.peer_addr);
                    esp_err_t result = esp_now_send(controller.peer_addr, (uint8_t *) &myMessage,
                                                    sizeof(myMessage) + 2);
                    if (result != ESP_OK) {
                        ESP_LOGI(TAG, "Sending %s to %s failed", myMessage.content, controllerMac.c_str());
                        const char *error_name = esp_err_to_name(result);
                        ESP_LOGI(TAG, "Error Code: %s", error_name);
                    }
                }
            }
            delete doc;
        }
    }
}
static bool introduceToController(PTDdevice* device) {
    auto* doc = new StaticJsonDocument<jsonDocumentSize>();
    (*doc)["action"] = "handshake"; // TODO use Enum type.
    (*doc)["type"] = device->type;
    return sendToController(doc);
}

bool sendToController(StaticJsonDocument<jsonDocumentSize>* doc) {
    if( xQueueSend(sendQueue, &doc, 10) != pdPASS) {
        ESP_LOGI(TAG, "failed to submit doc to sendQueue");
        return false;
    }
    return true;
}

static void addToReceiveQueue(clientMessage newMessage) {
    auto* doc = new StaticJsonDocument<jsonDocumentSize>();
    DeserializationError error = deserializeJson((*doc), newMessage.content);
    if (!error) {
        if( xQueueSend(receiveQueue, &doc, 10) != pdPASS) {
            ESP_LOGI(TAG, "Failed to submit to receiveQueue");
        }
    } else {
        ESP_LOGE(TAG, "deserializeJson() failed: %s", error.c_str());
    }
}

static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    clientMessage incomingMessage = {};
    memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    addToReceiveQueue(incomingMessage);
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
    esp_now_register_recv_cb(&OnDataRecv);

    int16_t scanResults = WiFi.scanNetworks();
    if (scanResults == 0) {
        ESP_LOGI(TAG, "No WiFi devices in AP Mode found");
        return false;
    }
    ESP_LOGI(TAG,"Found %d devices", scanResults);
    BaseType_t createReceive = xTaskCreate(processReceiveQueue,
                "Reading",
                2048,
                NULL,
                0,
                NULL);
    ESP_LOGI(TAG, "creating processReceiveQueue %s", (createReceive == pdPASS) ? "success" : "fail");

    BaseType_t createSend = xTaskCreate(processSendQueue,
                                        "Sending",
                                        4096,
                                        NULL,
                                        0,
                                        NULL);
    ESP_LOGI(TAG, "creating processSendQueue %s", (createSend == pdPASS) ? "success" : "fail");

    for (int i = 0; i < scanResults; ++i) {
        String SSID = WiFi.SSID(i);
        String BSSIDstr = WiFi.BSSIDstr(i);
        if (SSID.indexOf("PTDController") == 0) {
            ESP_LOGI(TAG, "Device %d name is %s", i, SSID.c_str());
            esp_now_peer_info_t controller = {};
            controller.channel = CHANNEL;
            controller.encrypt = ENCRYPT;
            string2Mac(BSSIDstr, controller.peer_addr);
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