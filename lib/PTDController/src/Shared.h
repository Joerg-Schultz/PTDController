//
// Created by Joerg on 21.03.2023.
//

#ifndef PTDCONTROLLER_SHARED_H
#define PTDCONTROLLER_SHARED_H

#define CHANNEL 1
#define ENCRYPT false

struct clientMessage {
    char content[64];
};

struct PTDdevice {
    String macAddress;
    String type;
};

inline String mac2String(const uint8_t *mac) {
    String macString;
    for (byte i = 0; i < 6; ++i) {
        char buf[3];
        sprintf(buf, "%02X", mac[i]); // J-M-L: slight modification, added the 0 in the format for padding
        macString += buf;
        if (i < 5) macString += ':';
    }
    return macString;
}

inline void string2Mac(const String& macString, uint8_t target[]) {
    int mac[6];
    if (6 ==
        sscanf(macString.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4],
               &mac[5])) {
        for (int ii = 0; ii < 6; ++ii) {
            target[ii] = (uint8_t) mac[ii];
        }
    }
}
#endif //PTDCONTROLLER_SHARED_H
