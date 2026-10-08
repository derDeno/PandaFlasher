#pragma once
#include <Arduino.h>
#include <SD.h>

namespace SelfUpdate {
String install(File &file, void (*progress)(uint32_t, uint32_t));
}
