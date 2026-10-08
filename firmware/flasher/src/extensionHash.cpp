#include <Arduino.h>
#include <SD.h>
#include <mbedtls/md5.h>

namespace Extension {
String hashFile(File &file) {
    mbedtls_md5_context md5;
    mbedtls_md5_init(&md5);
    bool ok = mbedtls_md5_starts_ret(&md5) == 0;
    uint8_t chunk[512];
    uint32_t remaining = file.size();
    while (ok && remaining) {
        const size_t count = min<size_t>(sizeof(chunk), remaining);
        ok = file.read(chunk, count) == count && mbedtls_md5_update_ret(&md5, chunk, count) == 0;
        remaining -= count;
    }
    uint8_t digest[16];
    if (ok) ok = mbedtls_md5_finish_ret(&md5, digest) == 0;
    mbedtls_md5_free(&md5);
    if (!ok) return "";
    char result[33];
    for (unsigned i = 0; i < sizeof(digest); ++i) snprintf(result + i * 2, 3, "%02x", digest[i]);
    return result;
}
}
