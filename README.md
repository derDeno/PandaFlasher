# PandaFlasher

## 1. Project summary

PandaFlasher is an ESP32-S3 handheld programmer that flashes ESP targets over UART using its OLED screen, joystick, and SD card. An optional PandaExtension connects it to as many as eight targets for sequential discovery and flashing.

## 2. What the project does

### PandaFlasher

- **Flash from SD:** Select a `.bin` file from the SD card root and flash it at `0x0` (a merged image) or `0x10000` (an application image). The flasher checks the image and target capacity, shows progress, verifies the written data with MD5, and records each attempt in `/flash-log.txt`.
- **Target details:** Read the connected chip type, revision, flash size, MAC address, and available secure boot and flash encryption status.
- **Read Serial:** View printable target UART output on the OLED while raw output is saved to `/target-log.txt`.
- **UART Test:** Reset the connected target.
- **Firmware update:** Update PandaFlasher from an application image on the SD card. The update is written to the inactive OTA slot and checked before restart.
- **Software version:** View the installed PandaFlasher firmware version.

### PandaExtension

The optional rev 1 extension selects and powers one of eight ESP target ports at a time. PandaFlasher can discover ports, flash a selected port, or try **All responding ports** and report verified, failed, and no-response counts. The extension also reports its hardware revision, firmware version, MCU, MAC address, and port count, and accepts firmware updates from PandaFlasher. Extension boards cannot be chained.

## 3. Web UI: functions and connection

On PandaFlasher, choose **Settings → WiFi setup**. Use a phone or computer to scan the OLED QR code and join the password-protected `PandaFlasher-XXXX` access point, where `XXXX` is the last four hexadecimal characters of the Wi-Fi MAC address. You can rename the setup access point in **Web UI → Settings → Wi-Fi**. Open `http://192.168.4.1` and enter the Wi-Fi network credentials. After PandaFlasher joins that network, the setup access point closes. **Settings → WiFi status** shows its LAN address. From a device on the same network, open `http://<device-ip>` or `http://pandaflasher.local`. If that name is already taken, use `http://pandaflasher-xxxx.local`, where `xxxx` is the last four characters of the Wi-Fi MAC address.

The Web UI provides:

- **Home:** Identify connected ESP targets, select multiple responding targets, reset them, or flash them in sequence. With a PandaExtension connected, it can scan and operate its target ports too.
- **Filemanager:** Upload and delete `.bin` files on the PandaFlasher SD card.
- **Settings:** Change Wi-Fi, restart PandaFlasher or a connected PandaExtension, restart in Wi-Fi setup mode, enter deep sleep, and install OTA application images for either board.
- **Info:** View PandaFlasher version and uptime, network details, and connected PandaExtension details and version.

Web flashing erases all flash on each selected target and writes a merged full-flash image from `0x0`. The image must include the partition table at `0x8000` and application at `0x10000`; a standalone OTA application image is not suitable. The Web UI has no login, so anyone on the same network can use its controls. Deep sleep requires a power cycle to wake the device.

## 4. Connections and use

### PandaFlasher six-pin target connector

Pin numbers below refer to the PandaFlasher header. `RX` and `TX` are named from the PandaFlasher's perspective, so connect UART signals crossed: PandaFlasher RX to target TX, and PandaFlasher TX to target RX.

| Pin | Signal | Connect to target |
| ---: | --- | --- |
| 1 | GND | GND |
| 2 | RX | UART TX |
| 3 | TX | UART RX |
| 4 | 3.3 V | 3.3 V |
| 5 | BOOT | BOOT / IO0 |
| 6 | RESET | EN / RESET |

Use a 3.3 V ESP target and a common ground. The target UART runs at 115200 baud. If using a PandaExtension, connect its PandaFlasher header to this six-pin header instead of connecting one target directly. Power the extension from its USB-C 5 V input; it generates 3.3 V for the selected target. In this setup, PandaFlasher header pin 4 only drives the extension's presence LED and does not power the extension targets. Connect targets to the numbered extension ports. The extension's PWR OUT connector can pass its 5 V input to PandaFlasher.

Copy the target firmware `.bin` to the SD card root. Use joystick UP/DOWN to navigate and CENTER to select; choose the on-screen **Back** option with the joystick to cancel or return. Select **Flash from SD**, choose a file and offset, then press CENTER to start. Use `0x0` for a merged image; use `0x10000` for an application image only when a compatible bootloader and partition table are already installed on the target. Keep the target connected and powered until the result appears. With an extension, connect it before starting PandaFlasher because extension detection runs at startup; then choose a port or **All responding ports** in the flash workflow.

## 5. Releases

PandaFlasher and PandaExtension have independently versioned firmware and separate releases. A merged pull request from `dev` to `main` prepares a release for each board; either version can change without the other. Flasher release tags use `v<major>.<minor>.<patch>`, and extension release tags use `extension-v<major>.<minor>.<patch>`.

Each release contains two images:

- `*-OTA.bin` is the application image for an OTA update. Copy it to the SD card and choose the matching update function in PandaFlasher or the Web UI.
- `*-full.bin` combines the bootloader, partition table, application, and filesystem for a full USB/serial installation with Espressif `esptool`.

Flasher release files follow `PandaFlasher-<version>-OTA.bin` and `PandaFlasher-<version>-full.bin`. Extension release files follow `PandaFlasher-Extension-v<version>-OTA.bin` and `PandaFlasher-Extension-v<version>-full.bin`. Choose the release and file matching the board you are updating. Install the extension's full image over its own USB-C port the first time; then its OTA update can be sent through PandaFlasher. Do not use a full image for an OTA update.

## 6. Build both firmwares

Install Python and PlatformIO, then run these commands from the repository root.

### PandaFlasher (ESP32-S3, hardware revision 2.0)

```sh
python -m platformio run -d firmware/flasher -e HW-2_0
```

The application image is `firmware/flasher/.pio/build/HW-2_0/firmware.bin`. To build the Web UI filesystem too, run `python -m platformio run -d firmware/flasher -e HW-2_0 -t buildfs`. To upload over USB, run `python -m platformio run -d firmware/flasher -e HW-2_0 -t upload`.

### PandaExtension (ESP32-C3, rev 1)

```sh
python -m platformio run -d firmware/extension -e rev1
```

The application image is `firmware/extension/.pio/build/rev1/firmware.bin`. To upload over the extension's USB-C port, run `python -m platformio run -d firmware/extension -e rev1 -t upload` (add `--upload-port <port>` if PlatformIO does not find it automatically).
