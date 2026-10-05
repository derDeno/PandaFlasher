#pragma once

#include <Arduino.h>
#include <SD.h>
#include <stdarg.h>
#include <stdio.h>
#include "esp_loader.h"
#include "pinControl.h"

extern HardwareSerial TargetSerial;

namespace targetPort {
static uint32_t deadline;
static File *flashLog = nullptr;
static bool flashLogFailed = false;

static void log(esp_loader_port_t *, esp_loader_log_level_t level, const char *format, va_list args) {
    if (!flashLog) return;
    char message[160];
    vsnprintf(message, sizeof(message), format, args);
    const char *label = level == ESP_LOADER_LOG_ERROR ? "ERROR" : level == ESP_LOADER_LOG_WARN ? "WARN" : "INFO";
    if (!flashLog->printf("[%s] %s\n", label, message)) {
        flashLogFailed = true;
        flashLog = nullptr;
    }
}

static void enterBootloader(esp_loader_port_t *) {
    while (TargetSerial.available()) TargetSerial.read();
    PinControl::enterFlashMode();
}

static void reset(esp_loader_port_t *) { PinControl::reset(); }
static void startTimer(esp_loader_port_t *, uint32_t ms) { deadline = millis() + ms; }
static uint32_t remaining(esp_loader_port_t *) {
    int32_t left = static_cast<int32_t>(deadline - millis());
    return left > 0 ? left : 0;
}
static void wait(esp_loader_port_t *, uint32_t ms) { delay(ms); }
static esp_loader_error_t changeRate(esp_loader_port_t *, uint32_t rate) {
    TargetSerial.updateBaudRate(rate);
    return ESP_LOADER_SUCCESS;
}
static esp_loader_error_t write(esp_loader_port_t *, const uint8_t *data, uint16_t size, uint32_t) {
    if (TargetSerial.write(data, size) != size) return ESP_LOADER_ERROR_FAIL;
    TargetSerial.flush();
    return ESP_LOADER_SUCCESS;
}
static esp_loader_error_t read(esp_loader_port_t *, uint8_t *data, uint16_t size, uint32_t timeout) {
    uint32_t start = millis();
    for (uint16_t i = 0; i < size; ++i) {
        while (!TargetSerial.available()) {
            if (millis() - start >= timeout) return ESP_LOADER_ERROR_TIMEOUT;
            delay(1);
        }
        data[i] = TargetSerial.read();
    }
    return ESP_LOADER_SUCCESS;
}

static const esp_loader_port_ops_t ops = {
    nullptr, nullptr, enterBootloader, reset, startTimer, remaining, wait,
    log, nullptr, changeRate, write, read,
    nullptr, nullptr, nullptr, nullptr
};
}

struct TargetInfo {
    target_chip_t chip = ESP_UNKNOWN_CHIP;
    uint32_t flashSize = 0;
    uint8_t mac[6] = {};
    bool hasMac = false;
    uint16_t revision = 0;
    bool hasRevision = false;
    bool secureBoot = false;
    bool flashEncryption = false;
    bool hasSecurity = false;
};

class TargetFlasher {
public:
    esp_loader_error_t connect(bool withStub = true) {
        port_.ops = &targetPort::ops;
        esp_loader_error_t err = esp_loader_init_serial(&loader_, &port_);
        if (err != ESP_LOADER_SUCCESS) return err;
        initialized_ = true;
        esp_loader_connect_args_t args = ESP_LOADER_CONNECT_DEFAULT();
        return withStub ? esp_loader_connect_with_stub(&loader_, &args)
                        : esp_loader_connect(&loader_, &args);
    }

    void close(bool resetTarget = true) {
        if (!initialized_) return;
        if (resetTarget) PinControl::exitFlashMode();
        esp_loader_deinit(&loader_);
        initialized_ = false;
    }

    TargetInfo details() {
        TargetInfo info;
        info.chip = esp_loader_get_target(&loader_);
        esp_loader_flash_detect_size(&loader_, &info.flashSize);
        info.hasMac = esp_loader_read_mac(&loader_, info.mac) == ESP_LOADER_SUCCESS;
        info.hasRevision = esp_loader_get_chip_revision(&loader_, &info.revision) == ESP_LOADER_SUCCESS;
        esp_loader_target_security_info_t security = {};
        info.hasSecurity = esp_loader_get_security_info(&loader_, &security) == ESP_LOADER_SUCCESS;
        if (info.hasSecurity) {
            info.secureBoot = security.secure_boot_enabled;
            info.flashEncryption = security.flash_encryption_enabled;
        }
        return info;
    }

    esp_loader_error_t flash(File &file, uint32_t offset, void (*progress)(uint32_t, uint32_t)) {
        const uint32_t size = file.size();
        if (!size || size > UINT32_MAX - 3 || (offset & 3)) return ESP_LOADER_ERROR_INVALID_PARAM;
        uint32_t flashSize = 0;
        esp_loader_error_t err = esp_loader_flash_detect_size(&loader_, &flashSize);
        if (err != ESP_LOADER_SUCCESS) return err;
        const uint32_t padded = (size + 3) & ~3U;
        if (offset > flashSize || padded > flashSize - offset) return ESP_LOADER_ERROR_IMAGE_SIZE;

        uint8_t buffer[1024];
        esp_loader_flash_cfg_t cfg = {};
        cfg.offset = offset;
        cfg.image_size = padded;
        cfg.block_size = sizeof(buffer);
        err = esp_loader_flash_start(&loader_, &cfg);
        if (err != ESP_LOADER_SUCCESS) return err;
        progress(0, size);
        uint32_t written = 0;
        while (written < size) {
            uint32_t length = min<uint32_t>(sizeof(buffer), size - written);
            if (file.read(buffer, length) != length) return ESP_LOADER_ERROR_FAIL;
            uint32_t sendLength = (length + 3) & ~3U;
            memset(buffer + length, 0xFF, sendLength - length);
            err = esp_loader_flash_write(&loader_, &cfg, buffer, sendLength);
            if (err != ESP_LOADER_SUCCESS) return err;
            written += length;
            progress(written, size);
        }
        return esp_loader_flash_finish(&loader_, &cfg);
    }

private:
    esp_loader_t loader_ = {};
    esp_loader_port_t port_ = {};
    bool initialized_ = false;
};

inline const char *targetChipName(target_chip_t chip) {
    static const char *names[] = {
        "ESP8266", "ESP32", "ESP32-S2", "ESP32-C3", "ESP32-S3", "ESP32-C2",
        "ESP32-C5", "ESP32-H2", "ESP32-C6", "ESP32-P4", "ESP32-C61",
        "ESP32-S31", "ESP32-H21", "ESP32-H4"
    };
    return chip < ESP_MAX_CHIP ? names[chip] : "Unknown";
}

inline const char *targetErrorName(esp_loader_error_t error) {
    static const char *names[] = {
        "OK", "I/O error", "Timeout", "Image too large", "Verify failed",
        "Invalid input", "Invalid target", "Unsupported chip", "Unsupported", "Bad response"
    };
    return error <= ESP_LOADER_ERROR_INVALID_RESPONSE ? names[error] : "Loader error";
}
