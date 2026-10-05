# PandaFlasher-Prv

Firmware for hardware revision 2.0 (`../pcb/SCH.pdf`).

Flash a target through the six pin UART header: connect RX, TX, BOOT, EN, 3.3 V, and GND. Put an ESP image (`.bin`) in the SD card root. Choose **Flash from SD**, select the file, then select its flash offset: `0x0` for a merged image or `0x10000` for an application image whose partition starts there. The firmware checks the target flash size and verifies the written bytes with the target's MD5 before reporting success. **Chip details** shows the detected chip, revision, flash size, MAC, and available security flags.

The flasher uses [Espressif esp-serial-flasher](https://github.com/espressif/esp-serial-flasher) v2.1.0 (commit `57f55f51d7d9781a09f9843043aa9fa715b94654`, Apache-2.0). The vendored files are limited to its serial loader, bundled stubs, public/private headers, and license.

**Read Serial** appends the target UART output to `/target-log.txt` on the SD card. The file is closed when you press BACK. `SD ERR` on the OLED means logging is unavailable; serial viewing continues.
