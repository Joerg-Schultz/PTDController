//
// Created by Joerg on 17.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H

#include <WString.h>
#include <utility>
#include <vector>
#include <esp_now.h>

#define default_name "PTDController"

struct clientMessage {
    char content[64];
};

struct client {
    String macAddress;
    String type;
};

class Controller {
private:
    String name;
    String macAddress;
    client clients[ESP_NOW_MAX_TOTAL_PEER_NUM];
public:
    static QueueHandle_t msg_queue; //Class variable not happy to have it public...

public:
    explicit Controller(String controller_name);

    Controller();

    String getName();
    String getMacAddress();

    String start();

private:
    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

    static void addToQueue(const uint8_t *mac, clientMessage input);
};


#endif //PTDCONTROLLER_CONTROLLER_H
