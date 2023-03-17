//
// Created by Joerg on 17.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H

#include <WString.h>
#include <utility>

class Controller {
private:
    String name;
    String macAddress;
public:
    explicit Controller(String controller_name) {
        name = std::move(controller_name);
    }

    String getName() {
        return name;
    }
    String getMacAddress() {
        return macAddress;
    }

    String start();
};


#endif //PTDCONTROLLER_CONTROLLER_H
