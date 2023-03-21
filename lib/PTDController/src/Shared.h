//
// Created by Joerg on 21.03.2023.
//

#ifndef PTDCONTROLLER_SHARED_H
#define PTDCONTROLLER_SHARED_H

#define CHANNEL 1

struct clientMessage {
    char content[64];
};

struct PTDdevice {
    String macAddress;
    String type;
};

String mac2String(const uint8_t * mac);

#endif //PTDCONTROLLER_SHARED_H
