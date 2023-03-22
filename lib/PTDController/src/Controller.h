//
// Created by Joerg on 21.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H
#include "ArduinoJson.h"
#include "Shared.h"
const int jsonDocumentSize = 512;

void startController(PTDdevice * controller);
void readFromDeviceInQueue(void * parameters);

#endif //PTDCONTROLLER_CONTROLLER_H
