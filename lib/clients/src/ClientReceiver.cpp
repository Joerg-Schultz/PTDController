//
// Created by Joerg on 17.03.2023.
//

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include "ClientReceiver.h"

static const char* TAG = "Receiver";

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

    if (esp_now_init() == ESP_OK) {
        ESP_LOGI(TAG,"ESPNow Init Success");
    } else {
        ESP_LOGI(TAG,"ESPNow Init Failed");
    }
    return macAddress;
}

bool ClientReceiver::searchController() {
    int16_t scanResults = WiFi.scanNetworks();
    if (scanResults == 0) {
        ESP_LOGI(TAG, "No WiFi devices in AP Mode found");
        return false;
    }
    // ESP_LOGI(TAG,"Found %s devices", scanResults);
    ESP_LOGI(TAG,"Found %d devices", scanResults);
    for (int i = 0; i < scanResults; ++i) {
        String SSID = WiFi.SSID(i);
        String BSSIDstr = WiFi.BSSIDstr(i);
        ESP_LOGI(TAG, "Device %d name is %s", i, SSID.c_str());
        if (SSID.indexOf("PTDController") == 0) {
            ESP_LOGI(TAG,"Is Controller");
            esp_now_peer_info_t controller = {};
            controller.channel = 1;
            controller.encrypt = 0;
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
            } else {
                ESP_LOGI(TAG, "Pair fail");
            }
        }
    }
    return true;
}
