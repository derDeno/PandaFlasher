# PandaFlasher
Standalone ESP32 Flashing and Debugging Tool

PandaFlasher is a standalone, ESP32-S3–based device designed to flash, debug, and monitor custom microcontroller boards — without requiring a PC. It provides a portable and efficient solution for embedded developers working with ESP32-based systems and similar architectures.

The core functionality centers around a microSD card slot that allows users to store multiple firmware files and retrieve logs or crash data from the connected target device. An onboard OLED display, paired with a 5-way joystick, offers an intuitive interface for navigating options, executing tasks, and viewing real-time logs or flashing status directly on the device.

PandaFlasher includes a USB-C port for powering and flashing the master device itself, along with a 3.3V voltage regulator that reliably powers both the master and connected slave board. A male pin header facilitates a direct connection to the slave device, while dedicated RX/TX LEDs provide immediate visual feedback on serial communication. A power LED confirms active system status at a glance.

With its self-contained design and field-ready form factor, PandaFlasher is built for developers who need reliable flashing and debugging capabilities on the go—without tethering to a computer or relying on network connectivity.

## Key Components
- ESP32 S3
- SSD1315 based 0.96 OLED display
- USB-C connector
- MicroSD Card slot
- 5-Way Switch
- 3.3V Voltage regulator
- RX / TX LEDs
- Power LED


# PCB
PandaFlasher is open sourced and can be founnd here on [OSHWLab](https://oshwlab.com/derdeno/pandaflasher).
You will find there all needed informations regarding parts, pcb design und production files.
YOu can also order a pcb from JLCPCB using OSHWLab's website.


# Supporting this Project
![](https://derdeno.github.io/PandaGarage/assets/images/sponsor_easyeda-bfd174b4cf605ca9d4454b99b5a4394e.png)
![](https://derdeno.github.io/PandaGarage/assets/images/sponsor_jlcpcb-889f0bca2654b6b4ac336c8d4a51f32d.png)

This project is currently sponsored by JLCPCB / EasyEDA, but welcomes additional support from the community. 

If you're interested in supporting this project, you can:

- Reach out on Discord [here](https://discord.gg/8VhnsCXKun)

Your contributions and support are greatly appreciated!