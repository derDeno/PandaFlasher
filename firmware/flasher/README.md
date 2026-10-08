# PandaFlasher firmware

PandaFlasher is firmware for an ESP32-S3 handheld programmer. It uses an OLED screen, joystick, back button, and SD card to flash another ESP chip over UART without a computer. This firmware is configured for PandaFlasher hardware revision 2.0.

## What it does

- **Flash from SD:** Lists up to 20 `.bin` files in the SD card root. Select a file and choose `0x0` for a merged image or `0x10000` for an application image built for a partition at that address. The firmware checks the ESP image header and target flash capacity, shows progress, and compares the written image with the target using MD5 before reporting success.
- **Chip details:** Connects to the target and displays its detected chip type, revision, flash size, MAC address, and secure boot and flash encryption status when available.
- **Read Serial:** Shows printable target UART output on the OLED and appends the raw output to `/target-log.txt` on the SD card. Press BACK to close the log and return to the menu. If the card or log write fails, `SD ERR` appears on the display; serial viewing continues.
- **Flash log:** Each flash attempt appends the selected file, offset, loader messages, progress, and result to `/flash-log.txt` on the SD card. Flashing continues if the log cannot be written, and the result screen reports `Log unavailable`.
- **UART Test:** Pulses the target reset line when CENTER is pressed.
- **Software Version:** Displays the firmware version.
- **Extension board:** At startup, probes for a rev 1 extension for up to two seconds. When found, `EXT ACTIVE` appears on the main menu. After choosing a `.bin` file, select one of ports 1–8 or `All responding ports`, then choose the flash offset. In the all-port mode, the flasher powers and probes each port in order, flashes targets that answer, and reports verified, failed, and no-response counts. It powers each port off before moving to the next one. Detection happens only at startup; reconnecting the extension requires restarting the PandaFlasher.
- **Extension status:** When the extension is detected, open **Extension → Status** to see its hardware revision, firmware version, MCU, MAC address, and number of ports from the startup identification reply.
- **Extension update:** Choose **Extension → Update firmware** to send a C3 application `.bin` from SD to the extension's inactive OTA slot. The image must fit its 1.25 MB slot. The extension checks its MD5 before rebooting; the flasher then refreshes its identification. Install the OTA-capable extension firmware over the extension's USB-C port once before using this menu item.
- **PandaFlasher update:** Choose **Settings → Update Panda**, select a PandaFlasher ESP32-S3 application `.bin` from SD, and confirm. The firmware validates the image type, writes the inactive OTA slot, checks the image MD5, and restarts. The included 8 MB partition table provides two 2.94 MB OTA app slots. Keep power connected until the device restarts. Use the application `firmware.bin` produced by the `HW-2_0` build, not a merged flash image.
- **Wi-Fi and web UI:** Choose **Settings → WiFi setup**. Scan the OLED QR code to join the password-protected `Panda-xxxxxx` access point, then open `http://192.168.4.1` and enter your Wi-Fi network. **Settings → WiFi status** shows the device's LAN IP. Open `http://<device-ip>` or `http://pandaflasher.local` on the same network. If that mDNS name is already in use, open `http://pandaflasher-xxxx.local`, where `xxxx` is the last four characters of the Wi-Fi MAC address; **Info → Device name** shows the active name. The web UI identifies connected targets, lets you select several ports, and resets or flashes each selected target. Its Filemanager uploads and deletes SD `.bin` files. Settings provides Wi-Fi setup, restart, deep sleep, and OTA updates for PandaFlasher and a connected PandaExtension. Info shows live device, network, and PandaExtension details. Web flashing accepts a merged full-flash image with a partition table at `0x8000` and ESP application at `0x10000`; it erases the target's entire flash before writing at `0x0`. The setup AP closes after the device joins the LAN. Anyone with access to that LAN can use the web controls.

## Connection and use

Connect the target's **RX, TX, BOOT/IO0, EN, 3.3 V, and GND** to the matching six-pin UART connection. Use a 3.3 V ESP target and connect RX to TX in each direction. The firmware controls BOOT/IO0 and EN to enter the target's serial bootloader and reset it afterward. Target UART communication runs at 115200 baud.

With the extension board, connect its PandaFlasher header instead of a target cable and power the extension from its own 5 V input. The extension powers the selected target; the PandaFlasher 3.3 V header pin only lights the extension's presence LED. A port reported as `no response` may be empty, unpowered, incompatible, or faulty.

Copy an ESP firmware `.bin` file to the SD card root. Use joystick UP/DOWN to navigate, CENTER to select, LEFT/RIGHT to change the flash offset on the confirmation screen, and BACK to cancel or return. Select **Flash from SD**, choose the file and correct offset, then press CENTER to start. Keep the target connected until the result appears. A standalone application image at `0x10000` also requires a compatible bootloader and partition table already on the target; a merged image at `0x0` includes those components if it was built that way.

## Build

This directory is a PlatformIO project. Build the `HW-2_0` environment with `pio run -e HW-2_0`, then upload with `pio run -e HW-2_0 -t upload`. It targets `esp32-s3-devkitc-1` with the Arduino framework, 8 MB flash, PSRAM, and the included partition layout. The display uses an SSD1306-compatible I2C driver; the SD card uses SPI. Pin assignments and the displayed version are in `src/config.h`.

The firmware includes Espressif's [esp-serial-flasher](https://github.com/espressif/esp-serial-flasher) v2.1.0 (commit `57f55f51d7d9781a09f9843043aa9fa715b94654`, Apache-2.0) for target detection, flashing, and verification. See `LICENSE` for the firmware license and `lib/esp-serial-flasher/LICENSE` for the bundled library license.


## PCB

The Schematic and Gerber files are open source and available at [OSHWLAB](https://oshwlab.com/derdeno/pandaflasher).

The Enclousure can be found here [Maker World](https://makerworld.com/de/@dencel)


## Future of this Project

The web UI is experimental. Flash and update operations run synchronously; the browser waits for their result while the OLED shows progress. The PandaExtension restart button and uptime reading require the updated PandaExtension firmware. Shutdown enters ESP32 deep sleep and requires a power cycle to wake. Hardware behavior and QR scanning still need validation on a PandaFlasher board.
