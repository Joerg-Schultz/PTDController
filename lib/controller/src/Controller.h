//
// Created by Joerg on 17.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H

#include <WString.h>
#include <utility>
#include <vector>

#define default_name "PTDController"

struct message {
    String content;
};

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

    static void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
};


#endif //PTDCONTROLLER_CONTROLLER_H
