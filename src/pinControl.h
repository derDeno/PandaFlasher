#pragma once
#include "Config.h"
#include <Arduino.h>

class PinControl {
public:
  static void init() {
    pinMode(ESP_IO0, OUTPUT);
    pinMode(ESP_EN, OUTPUT);
    // Idle: IO0=LOW, EN=LOW
    digitalWrite(ESP_IO0, LOW);
    digitalWrite(ESP_EN, LOW);
  }

  // Hold IO0 HIGH, pulse EN high→low → target enters UART‐bootloader
  static void enterFlashMode() {
    digitalWrite(ESP_IO0, HIGH);
    digitalWrite(ESP_EN, HIGH);
    delay(50);
    digitalWrite(ESP_EN, LOW);
    delay(100);
    digitalWrite(ESP_IO0, LOW);
  }

  // Release IO0, pulse EN to reboot back into normal mode
  static void exitFlashMode() {
    digitalWrite(ESP_EN, HIGH);
    digitalWrite(ESP_IO0, LOW);
    delay(50);
    digitalWrite(ESP_EN, LOW);
    delay(50);
  }


  static void reset() {
    digitalWrite(ESP_EN, HIGH);
    delay(100);
    digitalWrite(ESP_EN, LOW);
    delay(100);
  }
};
