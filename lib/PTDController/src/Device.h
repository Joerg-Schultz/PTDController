//
// Created by Joerg on 22.03.2023.
//

#ifndef PTDCONTROLLER_DEVICE_H
#define PTDCONTROLLER_DEVICE_H
#include "ArduinoJson.h"
#include "Shared.h"
const int jsonDocumentSize = 64;

bool startDevice(PTDdevice * device);
#endif //PTDCONTROLLER_DEVICE_H
