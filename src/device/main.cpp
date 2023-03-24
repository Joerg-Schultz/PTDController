#include <Arduino.h>
#include "Device.h"
#include "Shared.h"

PTDdevice treater = {"", "PTDTreater"};

void setup() {
    Serial.begin(115200);
    startDevice(&treater);
}

static StaticJsonDocument<jsonDocumentSize> report;
bool sniffing = false;
void loop() {
    delay(3000);
    report["action"] = "data";
    report["measurement"] = sniffing ? "Sniffing" : "Not sniffing";
    sendToController(report);
    sniffing = !sniffing;
}