#include <Arduino.h>
#include "pinControl.h"
#include "EspSerialFlasher.h"


void uartSetup() {
    pinMode(ESP_EN, OUTPUT);
    pinMode(ESP_IO0, OUTPUT);
    pinMode(ESP_TX, OUTPUT);
    pinMode(ESP_RX, INPUT);

    digitalWrite(ESP_EN, HIGH);
    digitalWrite(ESP_IO0, LOW);

    PinControl::init();

    // Serial2 ↔ target ESP32
    Serial2.begin(115200, SERIAL_8N1, ESP_TX, ESP_RX);
}

void uartFlash() {
    PinControl::enterFlashMode();

    File fw;
    if (!EspSerialFlasher::flashFirmware(fw)) {
        Serial.println("ERROR: Flash failed!");
    }

    // Reset back to normal
    PinControl::exitFlashMode();
}

