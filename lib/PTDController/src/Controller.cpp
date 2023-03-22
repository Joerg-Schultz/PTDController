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
QueueHandle_t deviceInQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t deviceOutQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t BTInQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t BTOutQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t pairingQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));

static const char* TAG = "Controller";

static PTDdevice controller = {"", "PTDController"};
static std::vector<PTDdevice> clientList = {};

static void addToDeviceInQueue(const uint8_t* mac, clientMessage newMessage) {
    StaticJsonDocument<jsonDocumentSize> doc;
    DeserializationError error = deserializeJson(doc, newMessage.content);
    if (!error) {
        doc["sender"] = mac2String(mac);
        String bla = doc["action"];
        ESP_LOGI(TAG, "sending to deviceInQueue %s", bla.c_str());
        if( xQueueSend(deviceInQueue, (void *)&doc, 10)) {
            ESP_LOGI(TAG, "Send successful");
        } else {
            ESP_LOGI(TAG, "Send failed");
        }

    } else {
        ESP_LOGE(TAG, "deserializeJson() failed: %s", error.c_str());
    }
}

static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    clientMessage incomingMessage = {};
    memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    addToDeviceInQueue(mac, incomingMessage);
}

void pairWithClient(void * parameters) {
    StaticJsonDocument<jsonDocumentSize> doc;
    while(1) {
        if (xQueueReceive(pairingQueue, (void *) &doc, 0) == pdTRUE) {
            esp_now_peer_info_t esp_now_client = {};
            esp_now_client.channel = CHANNEL;
            esp_now_client.encrypt = ENCRYPT;
            int mac[6];
            String sender = doc["sender"];
            if (6 == sscanf(sender.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4],
                            &mac[5])) {
                for (int i = 0; i < 6; ++i) {
                    esp_now_client.peer_addr[i] = (uint8_t) mac[i];
                }
                if (esp_now_is_peer_exist(esp_now_client.peer_addr)) {
                    ESP_LOGI(TAG, "Device %s already paired", sender.c_str());
                    continue;
                }
                esp_err_t addStatus = esp_now_add_peer(&esp_now_client);
                if (addStatus == ESP_OK) {
                    ESP_LOGI(TAG, "Pair with %s success", sender.c_str());
                    clientList.push_back({doc["sender"], doc["type"]});
                } else {
                    ESP_LOGI(TAG, "Pair with %s failed", sender.c_str());
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
    xTaskCreate(readFromDeviceInQueue,
                "Reading",
                4096,
                NULL,
                0,
                NULL);

    xTaskCreate(pairWithClient,
                "Pairing",
                4096,
                NULL,
                0,
                NULL);
 }