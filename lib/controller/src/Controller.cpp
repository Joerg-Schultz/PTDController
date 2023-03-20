//
// Created by Joerg on 17.03.2023.
//

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <vector>
#include "Controller.h"

static const char* TAG = "Controller";
#define CHANNEL 1

Controller::Controller(String controller_name) {
    name = std::move(controller_name);
}
Controller::Controller() {
    name = default_name;
}

String Controller::getMacAddress() {
    return macAddress;
}

void Controller::OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    message incomingMessage;
    ESP_LOGI(TAG, "Got message");
    memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
    ESP_LOGI(TAG, "Content: %s", incomingMessage);
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
    return macAddress;
}

String Controller::getName() {
    return name;
}
