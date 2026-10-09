"""Check that a built image can be sent through the extension OTA menu."""

from pathlib import Path
from struct import unpack_from
import sys

image = Path(sys.argv[1] if len(sys.argv) > 1 else "firmware/extension/.pio/build/rev1/firmware.bin").read_bytes()
assert 36 <= len(image) <= 0x140000, "image does not fit the OTA slot"
assert image[0] == 0xE9, "not an ESP application image"
assert unpack_from("<H", image, 12)[0] == 5, "not an ESP32-C3 image"
assert unpack_from("<I", image, 32)[0] == 0xABCD5432, "not an application image"
print("ESP32-C3 OTA application image OK")
