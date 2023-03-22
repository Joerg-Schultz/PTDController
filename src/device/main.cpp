#include <Arduino.h>
#include "Device.h"
#include "Shared.h"

PTDdevice treater = {"", "PTDTreater"};

void setup() {
    Serial.begin(115200);
    startDevice(&treater);
}

void loop() {
// write your code here
}