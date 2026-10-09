#pragma once
#include <Arduino.h>
#include <SD.h>
#include <esp_app_format.h>
#include "config.h"

extern HardwareSerial TargetSerial;

namespace Extension {
String hashFile(File &file);
static bool active = false;
static uint8_t selected = 0;
static char identity[144] = {};

static bool waitResponse(const char *expected, uint32_t timeoutMs) {
    char response[144];
    size_t length = 0;
    const uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (!TargetSerial.available()) { delay(1); continue; }
        const char c = TargetSerial.read();
        if (c == '\n') {
            response[length] = '\0';
            if (strcmp(expected, "OK ID ") == 0) {
                const bool valid = strncmp(response, expected, strlen(expected)) == 0 && strstr(response, " PORTS=8") != nullptr;
                if (valid) memcpy(identity, response, length + 1);
                return valid;
            }
            return strcmp(response, expected) == 0;
        }
        if (length < sizeof(response) - 1) response[length++] = c;
        else return false;
    }
    return false;
}

static bool command(const char *request, const char *expected, uint32_t timeoutMs = 750) {
    while (TargetSerial.available()) TargetSerial.read();
    TargetSerial.print(request);
    TargetSerial.flush();
    return waitResponse(expected, timeoutMs);
}

static void detect() {
    active = command("PFX1:ID\n", "OK ID ", 2000);
}

static bool discover() { return active && !selected && command("PFX1:DISCOVER\n", "OK DISCOVER"); }
static bool restart() { return active && !selected && command("PFX1:RESTART\n", "OK RESTART"); }

static bool select(uint8_t port) {
    if (!active || selected || port < 1 || port > 8) return false;
    char request[] = "PFX1:SELECT:0\n";
    char expected[] = "OK SELECT 0";
    request[12] = '0' + port;
    expected[10] = '0' + port;
    if (!command(request, expected)) {
        // A lost acknowledgement may still have selected a port; release it.
        command("PFX1:DESELECT\n", "OK DESELECT");
        return false;
    }
    selected = port;
    delay(100); // Let the switched 3.3 V rail settle before BOOT/EN and UART.
    return true;
}

static bool deselect() {
    if (!selected) return true;
    // The target transaction must already be closed before this command.
    if (!command("PFX1:DESELECT\n", "OK DESELECT")) return false;
    selected = 0;
    return true;
}

static String update(File &file, void (*progress)(uint32_t, uint32_t)) {
    if (!active || selected) return "Extension busy";
    const uint32_t size = file.size();
    if (!size || size > 0x140000) return "Image too large";
    esp_image_header_t header = {};
    uint32_t appMagic = 0;
    if (!file.seek(0) || file.read(reinterpret_cast<uint8_t *>(&header), sizeof(header)) != sizeof(header) ||
        !file.seek(sizeof(header) + sizeof(esp_image_segment_header_t)) ||
        file.read(reinterpret_cast<uint8_t *>(&appMagic), sizeof(appMagic)) != sizeof(appMagic) ||
        header.magic != ESP_IMAGE_HEADER_MAGIC || header.chip_id != ESP_CHIP_ID_ESP32C3 ||
        appMagic != ESP_APP_DESC_MAGIC_WORD) return "Use C3 app .bin";
    if (!file.seek(0)) return "SD seek failed";
    const String md5Text = hashFile(file);
    if (!md5Text.length()) return "SD read failed";
    if (!file.seek(0)) return "SD seek failed";
    const String request = "PFX1:UPDATE:" + String(size) + ":" + md5Text + "\n";
    if (!command(request.c_str(), "OK UPDATE", 3000)) return "Update rejected";

    uint8_t chunk[512];
    uint32_t sent = 0;
    while (sent < size) {
        const size_t count = min<size_t>(sizeof(chunk), size - sent);
        if (file.read(chunk, count) != count) return "SD read failed";
        if (TargetSerial.write(chunk, count) != count) return "UART write failed";
        TargetSerial.flush();
        sent += count;
        if (!waitResponse(sent == size ? "OK DONE" : "OK CHUNK", 5000)) return "Transfer failed";
        progress(sent, size);
    }
    return "Update sent!";
}
}
