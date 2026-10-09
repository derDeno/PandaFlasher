"""Check that a built PandaFlasher image fits its OTA slot."""

from pathlib import Path
from struct import unpack_from
import sys

image = Path(sys.argv[1] if len(sys.argv) > 1 else "firmware/flasher/.pio/build/HW-2_0/firmware.bin").read_bytes()
assert 36 <= len(image) <= 0x2F0000, "image does not fit the OTA slot"
assert image[0] == 0xE9, "not an ESP application image"
assert unpack_from("<H", image, 12)[0] == 9, "not an ESP32-S3 image"
assert unpack_from("<I", image, 32)[0] == 0xABCD5432, "not an application image"
print("ESP32-S3 OTA application image OK")
