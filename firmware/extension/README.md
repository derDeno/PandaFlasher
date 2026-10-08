# PandaFlasher extension board (rev 1)

The extension board lets one PandaFlasher connect to eight ESP target boards without moving the programming cable. Its MCU selects **one target at a time**. The selected target receives 3.3 V, UART, BOOT, and EN; the other seven target ports remain unpowered and disconnected from those four signals. Rev 1 has no extension-board chaining or inter-board discovery.

The hardware reference is [`boards/extension/pcb/`](../../boards/extension/pcb/). The firmware in this directory implements the extension side of the protocol below. The PandaFlasher firmware probes for the extension at startup and can flash one port or all responding ports from its flash menu.

## Build and upload

Run `python -m platformio run -d firmware/extension -e rev1` from the repository root. Upload over the extension MCU's USB-C connector with `python -m platformio run -d firmware/extension -e rev1 -t upload` (specify `--upload-port` if needed). This first USB upload installs the OTA-capable firmware; older extension firmware cannot accept an update over the PandaFlasher header. The project targets the 4 MB ESP32-C3-MINI-1U-N4 with the Arduino framework. Its included `partitions-4mb.csv` provides two 1.25 MB OTA app slots. The reported firmware version is in `src/main.cpp`.

For later updates, copy the **application image** `firmware/extension/.pio/build/rev1/firmware.bin` to the PandaFlasher SD card root and use **Extension → Update firmware**. Keep both boards powered until the result screen appears. A merged image or firmware for a different chip is rejected. An interrupted transfer leaves the currently running app selected. The flasher checks the new extension identity after it restarts; if it does not return in time, restart the PandaFlasher.

At startup all target ports are off and the extension's `PF-RX` driver is disconnected. The firmware accepts commands only on GPIO3 at 115200 baud. A selected target stays powered until `DESELECT` or an extension reset. After `SELECT`, wait for its rail to settle before starting a target transaction. Do not send `DESELECT` during an active flash operation.

## Hardware and power

- Extension MCU: **ESP32-C3-MINI-1U-N4** (`U18`), powered at 3.3 V by the extension board.
- A 5 V USB-C input feeds the local **TPS62162DSGR** (`U9`), which generates the extension's 3.3 V rail. The rail supplies U18, the selector logic, and the selected target. U9 is rated for 1 A total output; the eight target ports do not each get a separate 1 A supply.
- The **PWR OUT** USB-C connector passes the input 5 V to the PandaFlasher. It is intended only as a power output to the PandaFlasher.
- PandaFlasher header pin 4 carries the PandaFlasher's own 3.3 V only to a 560 Ω resistor and the `PF` presence LED. It does **not** join the extension's 3.3 V rail or power the targets. All boards share GND.
- `U19` (74HC238) decodes the selected target. `U1`–`U8` (TPS22919) switch target power, and `U10`–`U17` (TMUX1511) switch each target's UART, BOOT, and EN lines. All five connections for a target use the same `SEL-Tn` signal.

## Connector signals

`RX` and `TX` are named from the **PandaFlasher's perspective** throughout this board. `PF-TX` carries data sent by the PandaFlasher; `PF-RX` carries data returned to it. The target's own UART RX/TX roles are opposite at the mating connection.

| PandaFlasher header pad | Signal | Extension-board use |
| --- | --- | --- |
| 1 | GND | Shared reference |
| 2 | `PF-RX` | Selected target's return UART data; extension MCU reply path through R13 |
| 3 | `PF-TX` | UART data to the selected target and input to extension MCU GPIO3 |
| 4 | PandaFlasher 3.3 V | Presence LED only |
| 5 | `IN-BOOT` | Switched to selected target BOOT |
| 6 | `IN-EN` | Switched to selected target EN |

Each target port has the same nets. The following numbers are the **extension PCB footprint pad numbers**. The target connectors are mounted in the opposite orientation; the PCB's numbered target pinout describes the mating orientation and should be followed when connecting a target.

| Extension target pad | Net on target `n` |
| --- | --- |
| 1 | `EN-OUTn` |
| 2 | `BOOT-OUTn` |
| 3 | `+3V3-Tn` |
| 4 | `TX-OUTn` (PandaFlasher TX signal) |
| 5 | `RX-OUTn` (PandaFlasher RX signal) |
| 6 | GND |

## ESP32-C3 pin assignment

GPIO numbers are the ESP32-C3 signal names; module-pad numbers identify pins on `U18` in the schematic. `RXD0` and `TXD0` also reach test points for local serial debugging.

| ESP32-C3 signal | U18 pad | Connection | Intended firmware role |
| --- | ---: | --- | --- |
| GPIO1 | 13 | R13 (1 kΩ) to `PF-RX` | Reply to PandaFlasher **only after all target switches are off**; otherwise high impedance |
| GPIO3 | 6 | `PF-TX` | Receive PandaFlasher commands; also observes traffic sent to a selected target |
| GPIO4 | 18 | `MUX-EN` | Enable the 74HC238; external 10 kΩ pulldown keeps all targets off at startup |
| GPIO5 | 19 | `MUX-A2` | Decoder address bit 2; 10 kΩ pulldown |
| GPIO6 | 20 | `MUX-A1` | Decoder address bit 1; 10 kΩ pulldown |
| GPIO7 | 21 | `MUX-A0` | Decoder address bit 0; 10 kΩ pulldown |
| GPIO9 | 24 | BOOT button | Local ESP32-C3 boot mode; not a target BOOT control |
| GPIO18 | 27 | USB D− through R7 (22 Ω) | Native USB programming/debugging |
| GPIO19 | 28 | USB D+ through R10 (22 Ω) | Native USB programming/debugging |
| EN | 8 | RESET button, pullup, capacitor | Local ESP32-C3 reset; not the target EN line |
| RXD0 / TXD0 | 30 / 31 | `RXD` / `TXD` test points | Optional local UART console |

GPIO0, GPIO2, GPIO8, and GPIO10 are currently unused. GPIO2 and GPIO8 are strapping pins; do not assign external control signals to them without checking boot behavior.

## Target selection

`U19` has its two active-low enables tied to GND and its active-high enable driven by `MUX-EN`. With `MUX-EN = 0`, every `SEL-Tn` output is low. When enabled, exactly one output is high. The outputs run in reverse target order:

| Target | Decoder output | `MUX-A2 A1 A0` |
| ---: | --- | --- |
| 1 | Y7 | `111` |
| 2 | Y6 | `110` |
| 3 | Y5 | `101` |
| 4 | Y4 | `100` |
| 5 | Y3 | `011` |
| 6 | Y2 | `010` |
| 7 | Y1 | `001` |
| 8 | Y0 | `000` |

To change targets, first drive `MUX-EN` low, then set the three address bits, then drive `MUX-EN` high. This prevents an intermediate address from briefly powering or connecting another target. After selection, allow the target 3.3 V rail to settle before the PandaFlasher starts its existing BOOT/EN sequence. Only one target may be enabled at a time.

## Communication flow

The PandaFlasher uses its existing 115200-baud, 8N1 target UART (`ESP_TX` = GPIO17, `ESP_RX` = GPIO16). Its target BOOT and EN outputs are GPIO18 and GPIO8. The extension board does not translate or re-time target UART data: the selected TMUX1511 passes the PandaFlasher's TX, RX, BOOT, and EN signals through to one target. The PandaFlasher continues to run the ESP serial-flashing protocol with that target.

The extension MCU listens on `PF-TX` for control commands. Both it and the selected target can hear PandaFlasher TX, so it must ignore ordinary target traffic. The rev-1 protocol uses 115200 baud, 8N1, ASCII, and a single LF (`\n`) terminator.

| PandaFlasher sends | Extension replies on `PF-RX` | Meaning |
| --- | --- | --- |
| `PFX1:ID\n` | `OK ID HW=1.0 MCU=ESP32-C3-MINI-1U-N4 FW=<version> MAC=<12-hex-digits> PORTS=8\n` | Identify this extension board. HW is the board revision, supplied by firmware configuration; the PCB has no revision-sense input. |
| `PFX1:DISCOVER\n` | `OK DISCOVER\n` | Start a PandaFlasher-driven scan of all eight ports, as described below. |
| `PFX1:SELECT:<n>\n`, `n` = 1–8 | `OK SELECT <n>\n` | Select and power one target. Valid only while no target is selected. |
| `PFX1:DESELECT\n` | `OK DESELECT\n` | Disconnect and power off the selected target. |
| `PFX1:UPDATE:<size>:<md5>\n` | `OK UPDATE\n` | With no target selected, begin an OTA application update. Size is decimal bytes and MD5 is 32 lowercase hex digits. The extension rejects images larger than its inactive OTA slot. |

After `OK UPDATE`, PandaFlasher sends the raw image in blocks of at most 512 bytes. The extension replies `OK CHUNK\n` after each non-final block and `OK DONE\n` after verifying the final block and activating the new OTA slot; it then restarts. `ERR IMAGE`, `ERR WRITE`, `ERR VERIFY`, or `ERR TIMEOUT` abort the update. The extension accepts only ESP32-C3 application images and checks the full-image MD5 before switching slots. No target port is powered during this transfer.

Every valid, understood command starts its response with `OK`. Invalid or unsupported commands return `ERR <reason>\n`; they must not change the selected port. An `OK` means the extension MCU received and accepted the command, not that a target was found or flashed successfully. PandaFlasher should time out if no valid reply arrives. The fixed `PFX1:` prefix distinguishes control messages from normal target traffic; when a target is selected, only recognize `DESELECT` after a UART idle gap and the complete exact line. If stronger protection against false matches proves necessary, add a checksum to both implementations before release.

On boot, GPIO1 is high impedance and `MUX-EN` is low. For `SELECT`, the extension sends its `OK` reply while every target is disconnected, then sets the address and enables the chosen target after the reply has finished transmitting. PandaFlasher waits for target power to settle before using its existing BOOT/EN and UART procedure. While a target is selected, extension GPIO1 stays high impedance. `DESELECT` is sent only after the target transaction has finished; the extension disables `MUX-EN` **before** sending `OK DESELECT` so the target cannot contend on `PF-RX`.

### Discovery sequence

1. PandaFlasher sends `PFX1:DISCOVER\n` with no target selected and waits for `OK DISCOVER\n`.
2. For each port 1 through 8, PandaFlasher sends `SELECT`, waits for `OK`, then uses its existing serial-flasher connection to reset and query that target's chip type, hardware/revision information, MAC address, and other supported details.
3. PandaFlasher records the result for that numbered port, ends the target UART transaction, sends `DESELECT`, and waits for `OK` before advancing to the next port.
4. A port with no target response is reported as `NO RESPONSE` (possibly empty). A silent, unpowered, incompatible, or faulty target cannot be distinguished from an empty socket with the current hardware.

The extension MCU cannot independently query target chips: GPIO3 only **receives** `PF-TX`, and its GPIO1 output returns data to the PandaFlasher on `PF-RX`. The PandaFlasher must generate each target's UART requests and interpret its replies. The extension controls port selection and power and acknowledges commands.

While a target is selected, `DESELECT` also reaches that target over `PF-TX`; send it only after flashing or information reads are finished. The extension must never drive `PF-RX` while a target is connected. Keep targets disconnected for `ID`, `DISCOVER`, and status replies, and return GPIO1 to high impedance before selecting the next target.

Rev 1 has no connection or addressing between extension boards. There is one PandaFlasher-to-extension control link and, once selected, one PandaFlasher-to-target serial link.
