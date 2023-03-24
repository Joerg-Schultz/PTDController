//
// Created by Joerg on 21.03.2023.
//

#ifndef PTDCONTROLLER_CONTROLLER_H
#define PTDCONTROLLER_CONTROLLER_H
#include "ArduinoJson.h"
#include "Shared.h"
const int jsonDocumentSize = 64;

void startController(PTDdevice * controller);
void processDeviceReceiveQueue(void * parameters);
void sendToDeviceViaType(const String& type, const StaticJsonDocument<jsonDocumentSize>& document);
bool sendToDeviceViaMac(const String& mac, StaticJsonDocument<jsonDocumentSize> document);
#endif //PTDCONTROLLER_CONTROLLER_H
