#include <Arduino.h>

#include "config.h"
#include "uart.h"
#include "extension.h"
#include "selfUpdate.h"
#include "targetFlasher.h"
#include "menu.h"


void setup() {

    Serial.begin(115200);
    Wire.begin(2, 1); // SDA, SCL

    setupMenu();
}


void loop() {
    loopMenu();
}
