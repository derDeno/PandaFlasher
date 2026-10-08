#include <Arduino.h>

#include "config.h"
#include "uart.h"
#include "extension.h"
#include "selfUpdate.h"
#include "targetFlasher.h"
#include "menu.h"
#include "wifiWeb.h"


void setup() {

    Serial.begin(115200);
    Wire.begin(2, 1); // SDA, SCL

    setupMenu();
}


void loop() {
    WifiWeb::loop();
    if (restartAtMs && static_cast<int32_t>(millis() - restartAtMs) >= 0) ESP.restart();
    loopMenu();
}
