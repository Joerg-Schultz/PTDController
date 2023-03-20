//
// Created by Joerg on 17.03.2023.
//

#ifndef PTDCONTROLLER_CLIENTRECEIVER_H
#define PTDCONTROLLER_CLIENTRECEIVER_H


#include <WString.h>
#include <utility>
#include <esp_now.h>

struct message {
    char content[64];
};

class ClientReceiver {
private:
    String type;
    String macAddress;
    esp_now_peer_info_t controller = {};

public:
    explicit ClientReceiver(String receiver_type);

    String getMacAddress();

    String start();

    bool searchController();

private:
    bool sendToController(char* jsonString);

    void introduceToController();
};


#endif //PTDCONTROLLER_CLIENTRECEIVER_H
