#include "selfUpdate.h"
#include <Update.h>
#include <esp_app_format.h>

namespace Extension { String hashFile(File &file); }

namespace SelfUpdate {
String install(File &file, void (*progress)(uint32_t, uint32_t)) {
    const uint32_t size = file.size();
    if (!size) return "Empty image";

    esp_image_header_t header = {};
    uint32_t appMagic = 0;
    if (!file.seek(0) || file.read(reinterpret_cast<uint8_t *>(&header), sizeof(header)) != sizeof(header) ||
        !file.seek(sizeof(header) + sizeof(esp_image_segment_header_t)) ||
        file.read(reinterpret_cast<uint8_t *>(&appMagic), sizeof(appMagic)) != sizeof(appMagic) ||
        header.magic != ESP_IMAGE_HEADER_MAGIC || header.chip_id != ESP_CHIP_ID_ESP32S3 ||
        appMagic != ESP_APP_DESC_MAGIC_WORD) return "Use S3 app .bin";

    if (!file.seek(0)) return "SD seek failed";
    const String md5 = Extension::hashFile(file);
    if (!md5.length() || !file.seek(0)) return "SD read failed";
    if (!Update.begin(size, U_FLASH) || !Update.setMD5(md5.c_str())) {
        Update.abort();
        return "OTA slot unavailable";
    }

    uint8_t buffer[1024];
    uint32_t written = 0;
    while (written < size) {
        const size_t count = min<size_t>(sizeof(buffer), size - written);
        if (file.read(buffer, count) != count) {
            Update.abort();
            return "SD read failed";
        }
        if (Update.write(buffer, count) != count) {
            Update.abort();
            return "OTA write failed";
        }
        written += count;
        progress(written, size);
    }
    return Update.end() ? "Update verified" : "OTA verify failed";
}
}
