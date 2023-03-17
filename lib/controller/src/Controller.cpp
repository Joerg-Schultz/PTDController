//
// Created by Joerg on 17.03.2023.
//

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
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

String Controller::start() {
    WiFi.mode(WIFI_MODE_AP);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    macAddress = WiFi.macAddress();
    String SSID = name + "_" + macAddress;

    bool wifiResult = WiFi.softAP(SSID.c_str(), "Slave_Password", CHANNEL, 0);
    String wifiMessage = !wifiResult ?
                         "AP Config failed." :
                           "AP Config Success. Broadcasting with AP: " + String(SSID);
    Serial.println(wifiMessage);

    bool espResult = esp_now_init();
    String espMessage = (espResult == ESP_OK) ? "ESPNow Init Success" : "ESPNow Init Failed";
    Serial.println(espMessage);
    return macAddress;
}

String Controller::getName() {
    return name;
}
