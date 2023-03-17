//
// Created by Joerg on 17.03.2023.
//

#ifndef PTDCONTROLLER_CLIENTRECEIVER_H
#define PTDCONTROLLER_CLIENTRECEIVER_H


#include <WString.h>
#include <utility>

class ClientReceiver {
private:
    String type;
    String macAddress;
public:
    explicit ClientReceiver(String receiver_type);

    String getMacAddress();

    String start();
};


#endif //PTDCONTROLLER_CLIENTRECEIVER_H
