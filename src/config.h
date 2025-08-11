#pragma once

// UART Pins
#define UART_BAUD_RATE 115200
#define ESP_EN 8
#define ESP_IO0 18
#define ESP_TX 17
#define ESP_RX 16

// Joystick pins
#define JOY_UP 4
#define JOY_DOWN 6
#define JOY_RIGHT 15
#define JOY_LEFT 5
#define JOY_CENTER 7

// OLED display config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Back button (IO0)
#define BACK_BUTTON 0

// SD card CS pin
#define SD_CS 48
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCLK 18 


// —————— Flash chip parameters ——————
// JEDEC ID for your flash
#define FLASH_JEDEC_ID          0xEF4017
// Total flash size in bytes (e.g. 8MB):
#define FLASH_SIZE_BYTES        (8 * 1024 * 1024UL)
// Erase block (sometimes called “block”) size, usually 64KB:
#define FLASH_BLOCK_SIZE        (64 * 1024UL)
// Erase sector size, usually 4KB:
#define FLASH_SECTOR_SIZE       (4 * 1024UL)
// Program page size, usually 256 bytes:
#define FLASH_PAGE_SIZE         256
// Status‐register mask (usually 0 for default):
#define FLASH_STATUS_MASK       0x0