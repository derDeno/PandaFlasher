#pragma once
#include "Config.h"
#include <Arduino.h>

class PinControl {
public:
  static void init() {
    pinMode(ESP_IO0, OUTPUT);
    pinMode(ESP_EN, OUTPUT);
    // Idle: IO0=HIGH, EN=HIGH
    digitalWrite(ESP_IO0, HIGH);
    digitalWrite(ESP_EN, HIGH);
  }

  // Hold IO0 low, pulse EN low→high → target enters UART‐bootloader
  static void enterFlashMode() {
    digitalWrite(ESP_IO0, LOW);
    digitalWrite(ESP_EN, LOW);
    delay(10);
    digitalWrite(ESP_EN, HIGH);
    delay(50);
  }

  // Release IO0, pulse EN to reboot back into normal mode
  static void exitFlashMode() {
    digitalWrite(ESP_IO0, HIGH);
    digitalWrite(ESP_EN, LOW);
    delay(10);
    digitalWrite(ESP_EN, HIGH);
    delay(50);
  }
};
