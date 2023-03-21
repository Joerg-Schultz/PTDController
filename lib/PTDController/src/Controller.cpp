//
// Created by Joerg on 21.03.2023.
//
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <vector>
#include "Shared.h"
#include "Controller.h"

static const int msg_queue_len = 5;
static const int jsonDocumentSize = 1024;
QueueHandle_t msg_queue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));

static const char* TAG = "Controller";

static PTDdevice controller = {"", "PTDController"};
static std::vector<PTDdevice> clientList = {};

static void addToQueue(const uint8_t* mac, clientMessage newMessage) {
    StaticJsonDocument<jsonDocumentSize> doc;
    DeserializationError error = deserializeJson(doc, newMessage.content);
    if (!error) {
        doc["sender"] = mac2String(mac);
        xQueueSend(msg_queue, (void *)&doc, 10);
    } else {
        ESP_LOGE(TAG, "deserializeJson() failed: %s", error.c_str());
    }
}

static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    clientMessage incomingMessage = {};
    memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    addToQueue(mac, incomingMessage);
}

bool pairWithClient(const String &sender) {
    esp_now_peer_info_t esp_now_client = {};
    esp_now_client.channel = 1;
    esp_now_client.encrypt = false;
    int mac[6];
    if (6 == sscanf(sender.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4],
                    &mac[5])) {
        for (int i = 0; i < 6; ++i) {
            esp_now_client.peer_addr[i] = (uint8_t) mac[i];
        }
        esp_err_t addStatus = esp_now_add_peer(&esp_now_client);
        if (addStatus == ESP_OK) {
            ESP_LOGI(TAG, "Pair success");
            return true;
        } else {
            ESP_LOGI(TAG, "Pair fail");
            return false;
        }
    }
    return false;
}

void readFromQueue(void * parameters) {
    StaticJsonDocument<jsonDocumentSize> doc;
    while(1) {
        if (xQueueReceive(msg_queue, (void *)&doc, 0) == pdTRUE) {
            String reportJson;
            serializeJson(doc, reportJson);
            ESP_LOGI(TAG, "From Queue: %s", reportJson.c_str());
            if (doc["action"] == "handshake") {
                if(pairWithClient(doc["sender"])) {
                    clientList.push_back({ doc["sender"], doc["type"] });
                }
            }
        }
    }
}

void startController() {
    WiFi.mode(WIFI_MODE_AP);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    controller.macAddress = WiFi.macAddress();
    String SSID = controller.type + "_" + controller.macAddress;

    bool wifiResult = WiFi.softAP(SSID.c_str(), "Slave_Password", CHANNEL, 0);
    String wifiMessage = !wifiResult ?
                         "AP Config failed." :
                         "AP Config Success. Broadcasting with AP: " + String(SSID);
    ESP_LOGI(TAG, "%s", wifiMessage.c_str());

    bool espResult = esp_now_init();
    String espMessage = (espResult == ESP_OK) ? "ESPNow Init Success" : "ESPNow Init Failed";
    ESP_LOGI(TAG, "%s", espMessage.c_str());
    esp_now_register_recv_cb(&OnDataRecv);

    xTaskCreate(readFromQueue,
                "Record",
                4096,
                NULL,
                0,
                NULL);
}