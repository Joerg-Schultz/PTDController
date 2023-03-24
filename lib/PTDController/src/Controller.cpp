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
QueueHandle_t deviceReceiveQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t deviceSendQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t BTReceiveQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t BTSendQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));
QueueHandle_t pairingQueue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));

static const char* TAG = "Controller";
std::vector<PTDdevice> deviceList = {};

static void addToDeviceReceiveQueue(const uint8_t* mac, clientMessage newMessage) {
    static StaticJsonDocument<jsonDocumentSize> doc;
    DeserializationError error = deserializeJson(doc, newMessage.content);
    if (!error) {
        String macTest = mac2String(mac);
        doc["sender"] = macTest.c_str();
        String action = doc["action"];
        String sender = doc["sender"];
        ESP_LOGI(TAG, "sending to deviceInQueue %s from %s", action.c_str(), sender.c_str());
        if( xQueueSend(deviceReceiveQueue, (void *)&doc, 10)) {
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
    addToDeviceReceiveQueue(mac, incomingMessage);
}

void pairWithClient(void * parameters) {
    StaticJsonDocument<jsonDocumentSize> pairingJson;
    while (1) {
        if (xQueueReceive(pairingQueue, (void *) &pairingJson, 0) == pdTRUE) {
            esp_now_peer_info_t esp_now_client = {};
            esp_now_client.channel = CHANNEL;
            esp_now_client.encrypt = ENCRYPT;
            int mac[6];
            String sender = pairingJson["sender"];
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
                    deviceList.push_back({pairingJson["sender"], pairingJson["type"]});
                } else {
                    ESP_LOGI(TAG, "Pair with %s failed", sender.c_str());
                }
            }
        }
    }
}

static void processDeviceSendQueue(void* parameters) {
    StaticJsonDocument<jsonDocumentSize> sendJson;
    while (1) {
        if (xQueueReceive(deviceSendQueue, (void *) &sendJson, 0) == pdTRUE) {
            String targetMacString = sendJson["target"];
            sendJson.remove("target");
            int mac[6];
            sscanf(targetMacString.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4],
                   &mac[5]);
            clientMessage myMessage = {};
            ESP_LOGI(TAG,"Processing Sending task");
            serializeJson(sendJson,myMessage.content);
            ESP_LOGI(TAG, "Sending %s to %s", myMessage.content, targetMacString.c_str());
            esp_err_t result = esp_now_send((uint8_t *) mac, (uint8_t *) &myMessage, sizeof(myMessage) + 2);  //Sending "jsondata"
            ESP_LOGI(TAG, "Sending: %s", (result == ESP_OK) ? "Success" : "Failed");
        }
    }
}

void startController(PTDdevice * controller) {
    WiFi.mode(WIFI_MODE_AP);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    controller->macAddress = WiFi.macAddress();
    String SSID = controller->type + "_" + controller->macAddress;

    bool wifiResult = WiFi.softAP(SSID.c_str(), "Slave_Password", CHANNEL, 0);
    String wifiMessage = !wifiResult ?
                         "AP Config failed." :
                         "AP Config Success. Broadcasting with AP: " + String(SSID);
    ESP_LOGI(TAG, "%s", wifiMessage.c_str());

    bool espResult = esp_now_init();
    String espMessage = (espResult == ESP_OK) ? "ESPNow Init Success" : "ESPNow Init Failed";
    ESP_LOGI(TAG, "%s", espMessage.c_str());
    esp_now_register_recv_cb(&OnDataRecv);
    xTaskCreate(processDeviceReceiveQueue,
                "processDeviceReceive",
                4096,
                NULL,
                0,
                NULL);

    xTaskCreate(processDeviceSendQueue,
                "processDeviceSend",
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

void sendToDeviceViaType(const String& type, const StaticJsonDocument<jsonDocumentSize>& document) {
    for (const PTDdevice& device : deviceList) {
        String currentType = device.type;
        ESP_LOGI(TAG, "device %s", currentType.c_str());
        if (device.type == type) sendToDeviceViaMac(device.macAddress, document);
    }
}

bool sendToDeviceViaMac(const String& macAddress, StaticJsonDocument<jsonDocumentSize> document) {
    document["target"] = macAddress;

    if( xQueueSend(deviceSendQueue, (void *)&document, 10) == pdTRUE) {
        ESP_LOGI(TAG, "submitted doc to queue");
        return true;
    } else {
        ESP_LOGI(TAG, "failed to submit doc to queue");
        return false;
    }
}
