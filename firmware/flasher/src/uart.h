#include <Arduino.h>
#include "pinControl.h"

HardwareSerial TargetSerial(1);

void uartSetup() {
    pinMode(ESP_EN, OUTPUT);
    pinMode(ESP_IO0, OUTPUT);
    pinMode(ESP_TX, OUTPUT);
    pinMode(ESP_RX, INPUT);

    digitalWrite(ESP_EN, LOW);
    digitalWrite(ESP_IO0, LOW);

    PinControl::init();

    // Serial1 ↔ target ESP32
    // ponytail: Synchronous SD writes can overrun 2 KB on long stalls; add a logging queue if that occurs.
    TargetSerial.setRxBufferSize(2048);
    TargetSerial.begin(115200, SERIAL_8N1, ESP_RX, ESP_TX);
}

