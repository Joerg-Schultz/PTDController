//
// Created by Joerg on 21.03.2023.
//

#include <WString.h>
#include <Arduino.h>

String mac2String(const uint8_t * mac) {
    String macString;
    for (byte i = 0; i < 6; ++i) {
        char buf[3];
        sprintf(buf, "%02X", mac[i]); // J-M-L: slight modification, added the 0 in the format for padding
        macString += buf;
        if (i < 5) macString += ':';
    }
    return macString;
}