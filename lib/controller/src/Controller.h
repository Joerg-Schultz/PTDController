//
// Created by Joerg on 17.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H

#include <WString.h>
#include <utility>
#include <vector>

#define default_name "PTDController"

struct clientMessage {
    char content[64];
};

/*
struct jsonMessage {
    StaticJsonDocument<1024> messageDoc;
}; */

class Controller {
private:
    String name;
    String macAddress;
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
