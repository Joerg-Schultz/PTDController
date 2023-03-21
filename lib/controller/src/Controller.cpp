//
// Created by Joerg on 17.03.2023.
//

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <vector>
#include <ArduinoJson.h>
#include "Controller.h"

static const char* TAG = "Controller";
#define CHANNEL 1
static const int msg_queue_len = 5;
static const int jsonDocumentSize = 1024;
QueueHandle_t Controller::msg_queue = xQueueCreate(msg_queue_len, sizeof(StaticJsonDocument<jsonDocumentSize>));

String mac2String(const uint8_t * mac) {
    String macString;
    for (byte i = 0; i < 6; ++i)
    {
        char buf[3];
        sprintf(buf, "%02X", mac[i]); // J-M-L: slight modification, added the 0 in the format for padding
        macString += buf;
        if (i < 5) macString += ':';
    }
    return macString;
}

Controller::Controller(String controller_name) {
    name = std::move(controller_name);
}
Controller::Controller() : Controller(default_name){
}

String Controller::getMacAddress() {
    return macAddress;
}

void Controller::addToQueue(const uint8_t* mac, clientMessage newMessage) {
    StaticJsonDocument<jsonDocumentSize> doc;
    DeserializationError error = deserializeJson(doc, newMessage.content);
    if (!error) {
        doc["sender"] = mac2String(mac);
        xQueueSend(msg_queue, (void *)&doc, 10);
    } else {
        ESP_LOGE(TAG, "deserializeJson() failed: %s", error.c_str());
    }
}

void Controller::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    clientMessage incomingMessage = {};
    memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    // as parallel RTOS function??
    addToQueue(mac, incomingMessage);

    /*
    // put this into main as reaction to type : introduction
    esp_now_peer_info_t client = {};
    client.channel = 1;
    client.encrypt = 0;
    for (int i = 0; i < 6; ++i) {
        client.peer_addr[i] = (uint8_t) mac[i];
    }
    esp_err_t addStatus = esp_now_add_peer(&client);
    if (addStatus == ESP_OK) {
        ESP_LOGI(TAG, "Pair success");
    } else {
        ESP_LOGI(TAG, "Pair fail");
    }
     */
}

void Controller::readFromQueue(void * parameters) {
    StaticJsonDocument<jsonDocumentSize> doc;
    while(1) {
        if (xQueueReceive(msg_queue, (void *)&doc, 0) == pdTRUE) {
            String reportJson;
            serializeJson(doc, reportJson);
            ESP_LOGI(TAG, "From Queue: %s", reportJson.c_str());
            }
        }
    }
}

String Controller::start() {
    WiFi.mode(WIFI_MODE_AP);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    macAddress = WiFi.macAddress();
    String SSID = name + "_" + macAddress;

    bool wifiResult = WiFi.softAP(SSID.c_str(), "Slave_Password", CHANNEL, 0);
    String wifiMessage = !wifiResult ?
                         "AP Config failed." :
                           "AP Config Success. Broadcasting with AP: " + String(SSID);
    ESP_LOGI(TAG, "%s", wifiMessage.c_str());

    bool espResult = esp_now_init();
    String espMessage = (espResult == ESP_OK) ? "ESPNow Init Success" : "ESPNow Init Failed";
    ESP_LOGI(TAG, "%s", espMessage.c_str());
    esp_now_register_recv_cb(&Controller::OnDataRecv);

    xTaskCreate(&Controller::readFromQueue,
                "Record",
                4096,
                NULL,
                0,
                NULL);
    return macAddress;
}

String Controller::getName() {
    return name;
}
