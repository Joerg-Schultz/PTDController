//
// Created by Joerg on 21.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H
#include "ArduinoJson.h"
const int jsonDocumentSize = 512;

void startController();
void readFromDeviceInQueue(void * parameters);

#endif //PTDCONTROLLER_CONTROLLER_H
