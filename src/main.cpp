#include <Arduino.h>

#include "config.h"
#include "menu.h"
#include "uart.h"


void setup() {

    Serial.begin(115200);
    Wire.begin(1, 2); // SDA, SCL

    setupMenu();
    uartSetup();
}


void loop() {
    loopMenu();
}