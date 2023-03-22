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

static const char* TAG = "Device";

static std::vector<esp_now_peer_info_t> controllerList = {};

static bool sendToController(char jsonString[64]) {
    clientMessage myMessage = {};
    memcpy(&myMessage.content, jsonString, sizeof(myMessage.content));
    for (esp_now_peer_info_t controller : controllerList) {
        esp_err_t result = esp_now_send(controller.peer_addr, (uint8_t *) &myMessage, sizeof(myMessage) + 2);  //Sending "jsondata"
        ESP_LOGI(TAG, "Send content: %s", myMessage.content);
        if (result != ESP_OK) return false;
    }
    return true;
}

static bool introduceToController(PTDdevice* device) {
    StaticJsonDocument<400> doc;
    doc["action"] = "handshake"; // TODO use Enum type.
    doc["type"] = device->type;
    char jsonData [64] = ""; // TODO #define the length of content
    serializeJson(doc, jsonData);
    ESP_LOGI(TAG, "%s", jsonData);
    return sendToController(jsonData);
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