# PandaFlasher firmware

PandaFlasher is firmware for an ESP32-S3 handheld programmer. It uses an OLED screen, joystick, back button, and SD card to flash another ESP chip over UART without a computer. This firmware is configured for PandaFlasher hardware revision 2.0.

## Releases and firmware versions

PandaFlasher and the optional PandaExtension have separate firmware versions and separate GitHub releases. The current source versions are **PandaFlasher v2.2.0** and **PandaExtension v1.2.0**; the devices also report their versions in the **Info** screen of the Web UI and the extension status menu. After a pull request from `dev` is merged into `main`, the release workflow prepares two releases: **Flasher v2.2.0** (tag `v2.2.0`) and **Extension v1.2.0** (tag `extension-v1.2.0`). The two versions can change independently.

Each release contains two images:

- `*-OTA.bin` is the application image only. Copy it to the PandaFlasher SD card to update through **Settings → Update Panda**, **Extension → Update firmware**, or the Web UI's **Settings → Firmware update** controls. The extension's first installation must be done over its own USB-C port with a full image; older extension firmware may not support OTA.
- `*-full.bin` combines the bootloader, partition table, application, and filesystem for that device. Use it for a complete USB/serial installation with Espressif's `esptool`; it is not the image to select for an OTA update.

The current Flasher release contains `PandaFlasher-v2.2.0-OTA.bin` and `PandaFlasher-v2.2.0-full.bin`. The Extension release contains `PandaFlasher-Extension-v1.2.0-OTA.bin` and `PandaFlasher-Extension-v1.2.0-full.bin`. Download the image from the matching release and check the device name and image type before installing.

## What it does

- **Flash from SD:** Lists up to 20 `.bin` files in the SD card root. Select a file and choose `0x0` for a merged image or `0x10000` for an application image built for a partition at that address. The firmware checks the ESP image header and target flash capacity, shows progress, and compares the written image with the target using MD5 before reporting success.
- **Chip details:** Connects to the target and displays its detected chip type, revision, flash size, MAC address, and secure boot and flash encryption status when available.
- **Read Serial:** Shows printable target UART output on the OLED and appends the raw output to `/target-log.txt` on the SD card. Press BACK to close the log and return to the menu. If the card or log write fails, `SD ERR` appears on the display; serial viewing continues.
- **Flash log:** Each flash attempt appends the selected file, offset, loader messages, progress, and result to `/flash-log.txt` on the SD card. Flashing continues if the log cannot be written, and the result screen reports `Log unavailable`.
- **UART Test:** Pulses the target reset line when CENTER is pressed.
- **Software Version:** Displays the firmware version.

## Connection and use

Connect the target's **RX, TX, BOOT/IO0, EN, 3.3 V, and GND** to the matching six-pin UART connection. Use a 3.3 V ESP target and connect RX to TX in each direction. The firmware controls BOOT/IO0 and EN to enter the target's serial bootloader and reset it afterward. Target UART communication runs at 115200 baud.

Copy an ESP firmware `.bin` file to the SD card root. Use joystick UP/DOWN to navigate, CENTER to select, LEFT/RIGHT to change the flash offset on the confirmation screen, and BACK to cancel or return. Select **Flash from SD**, choose the file and correct offset, then press CENTER to start. Keep the target connected until the result appears. A standalone application image at `0x10000` also requires a compatible bootloader and partition table already on the target; a merged image at `0x0` includes those components if it was built that way.

## Web UI

Set up Wi-Fi from **Settings → WiFi setup** on the PandaFlasher. Scan the OLED QR code to join its password-protected `Panda-xxxxxx` access point, then open `http://192.168.4.1` to enter your Wi-Fi network. Once connected, the setup access point closes. Find the LAN address in **Settings → WiFi status**, then open `http://<device-ip>` or `http://pandaflasher.local` from a device on the same network.

The home page scans for connected ESP targets (including ports on a PandaExtension), lets you select multiple responding targets, resets them, and flashes them in sequence. Web flashing uses a merged full-flash image, erases the selected target's entire flash, and writes from address `0x0`; a standalone OTA application image is not suitable. **Filemanager** uploads and deletes `.bin` files on the SD card. **Settings** changes Wi-Fi, restarts the PandaFlasher or extension, enters deep sleep, and updates either board from its OTA image. **Info** shows the PandaFlasher version and uptime, network details, and connected extension details and version.

The Web UI has no login, so anyone with access to the same network can use its controls.

## Build

This directory is a PlatformIO project. Build the `HW-2_0` environment with `pio run -e HW-2_0`, then upload with `pio run -e HW-2_0 -t upload`. It targets `esp32-s3-devkitc-1` with the Arduino framework, 8 MB flash, PSRAM, and the included partition layout. The display uses an SSD1306-compatible I2C driver; the SD card uses SPI. Pin assignments and the displayed version are in `src/config.h`.

The firmware includes Espressif's [esp-serial-flasher](https://github.com/espressif/esp-serial-flasher) v2.1.0 (commit `57f55f51d7d9781a09f9843043aa9fa715b94654`, Apache-2.0) for target detection, flashing, and verification. See `LICENSE` for the firmware license and `lib/esp-serial-flasher/LICENSE` for the bundled library license.

## Extension firmware

Build the ESP32-C3 extension project with `pio run -d firmware/extension -e rev1`. See [its README](firmware/extension/README.md) for USB upload and OTA update instructions.
