//
// Created by Joerg on 17.03.2023.
//

#include <WiFi.h>
#include <esp_wifi.h>
#include "ClientReceiver.h"

ClientReceiver::ClientReceiver(String receiver_type) {
    type = std::move(receiver_type);
}

String ClientReceiver::getMacAddress() {
    return macAddress;
}

String ClientReceiver::start() {
    WiFi.mode(WIFI_STA);
    //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html
    // If the Wi-Fi mode is SoftAP, the ifx should be WIFI_IF_AP.
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    macAddress = WiFi.macAddress();
    return macAddress;
}