#pragma once
#include "Config.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SD.h>

class EspSerialFlasher {
public:
  // Read the entire File & flash it at 0x0000
  static bool flashFirmware(File &fwFile) {
    Serial.println("==> SYNC");
    if (!sync())                { Serial.println("SYNC failed"); return false; }

    Serial.println("==> SPI_ATTACH");
    if (!spiAttach())           { Serial.println("SPI_ATTACH failed"); return false; }

    Serial.println("==> SPI_SET_PARAMS");
    if (!spiSetParams())        { Serial.println("SPI_SET_PARAMS failed"); return false; }

    uint32_t imgSize = fwFile.size();
    uint32_t pktSize = FLASH_DATA_SIZE;  // 16KB blocks
    uint32_t numPkts = (imgSize + pktSize - 1) / pktSize;

    Serial.printf("==> FLASH_BEGIN: img=%u bytes, pkts=%u, pktSize=%u\n",
                  imgSize, numPkts, pktSize);
    if (!flashBegin(imgSize, numPkts, pktSize, 0)) {
      Serial.println("FLASH_BEGIN failed"); return false;
    }

    // Send each FLASH_DATA packet
    uint8_t buf[FLASH_DATA_SIZE];
    for (uint32_t seq = 0; seq < numPkts; seq++) {
      size_t len = fwFile.read(buf, pktSize);
      if (!flashData(seq, buf, len)) {
        Serial.printf("FLASH_DATA failed at block %u\n", seq); 
        return false;
      }
      Serial.printf("  → block %u/%u\n", seq+1, numPkts);
    }

    Serial.println("==> FLASH_END");
    if (!flashEnd(0)) {
      Serial.println("FLASH_END failed"); return false;
    }
    Serial.println("** Flash completed successfully **");
    return true;
  }

private:
  // Command codes
  static constexpr uint8_t ESP_SYNC           = 0x08;
  static constexpr uint8_t ESP_SPI_ATTACH     = 0x0D;
  static constexpr uint8_t ESP_SPI_SET_PARAMS = 0x0B;
  static constexpr uint8_t ESP_FLASH_BEGIN    = 0x02;
  static constexpr uint8_t ESP_FLASH_DATA     = 0x03;
  static constexpr uint8_t ESP_FLASH_END      = 0x04;

  // Sizes
  static constexpr uint32_t FLASH_DATA_SIZE = 16 * 1024;  // 16KB

  // SLIP framing bytes
  static constexpr uint8_t SLIP_END     = 0xC0;
  static constexpr uint8_t SLIP_ESC     = 0xDB;
  static constexpr uint8_t SLIP_ESC_END = 0xDC;
  static constexpr uint8_t SLIP_ESC_ESC = 0xDD;

  // Send the 36-byte SYNC payload, then wait for a matching response
  static bool sync() {
    static const uint8_t SYNC_PAYLOAD[36] = {
      0x07,0x07,0x12,0x20,
      // then 32 × 0x55
      0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
      0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
      0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
      0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55
    };
    Serial2.setTimeout(1000);
    return sendCommand(ESP_SYNC,       SYNC_PAYLOAD, sizeof(SYNC_PAYLOAD))
        && receiveResponse(ESP_SYNC);
  }

  // Tell ROM loader which SPI interface (0 = default)
  static bool spiAttach() {
    uint8_t p[8] = {0,0,0,0,  0,0,0,0};
    return sendCommand(ESP_SPI_ATTACH, p, sizeof(p))
        && receiveResponse(ESP_SPI_ATTACH);
  }

  // Configure flash parameters
  static bool spiSetParams() {
    uint32_t params[6] = {
      FLASH_JEDEC_ID,
      FLASH_SIZE_BYTES,
      FLASH_BLOCK_SIZE,
      FLASH_SECTOR_SIZE,
      FLASH_PAGE_SIZE,
      FLASH_STATUS_MASK
    };
    // pack LE
    uint8_t p[24];
    for(int i=0;i<6;i++){
      uint32_t v = params[i];
      p[i*4+0] = v & 0xFF;
      p[i*4+1] = (v>>8)&0xFF;
      p[i*4+2] = (v>>16)&0xFF;
      p[i*4+3] = (v>>24)&0xFF;
    }
    return sendCommand(ESP_SPI_SET_PARAMS, p, sizeof(p))
        && receiveResponse(ESP_SPI_SET_PARAMS);
  }

  // FLASH_BEGIN: 4×32-bit: [image_size, num_packets, packet_size, flash_offset]
  static bool flashBegin(uint32_t imgSize, uint32_t numPkts,
                         uint32_t pktSize, uint32_t offset) {
    uint32_t args[4] = { imgSize, numPkts, pktSize, offset };
    uint8_t p[16];
    for(int i=0;i<4;i++){
      uint32_t v = args[i];
      p[i*4+0] = v & 0xFF;
      p[i*4+1] = (v>>8)&0xFF;
      p[i*4+2] = (v>>16)&0xFF;
      p[i*4+3] = (v>>24)&0xFF;
    }
    return sendCommand(ESP_FLASH_BEGIN, p, sizeof(p))
        && receiveResponse(ESP_FLASH_BEGIN);
  }

  // FLASH_DATA: 4×32-bit header [size, seq, 0,0] + data bytes
  static bool flashData(uint32_t seq, const uint8_t *data, size_t len) {
    uint32_t hdr[4] = { (uint32_t)len, seq, 0, 0 };
    size_t total = 16 + len;
    uint8_t *p = (uint8_t*)malloc(total);
    // pack header
    for(int i=0;i<4;i++){
      uint32_t v = hdr[i];
      p[i*4+0] = v & 0xFF;
      p[i*4+1] = (v>>8)&0xFF;
      p[i*4+2] = (v>>16)&0xFF;
      p[i*4+3] = (v>>24)&0xFF;
    }
    // copy data
    memcpy(p + 16, data, len);

    bool ok = sendCommand(ESP_FLASH_DATA, p, total)
           && receiveResponse(ESP_FLASH_DATA);
    free(p);
    return ok;
  }

  // FLASH_END: single 32-bit word (0=reboot, 1=run user code)
  static bool flashEnd(uint32_t reboot) {
    uint8_t p[4] = {
      (uint8_t)( reboot      &0xFF),
      (uint8_t)((reboot>>8)  &0xFF),
      (uint8_t)((reboot>>16) &0xFF),
      (uint8_t)((reboot>>24) &0xFF)
    };
    return sendCommand(ESP_FLASH_END, p, sizeof(p))
        && receiveResponse(ESP_FLASH_END);
  }

  // — SLIP‐encode & send a command packet header+payload —
  static bool sendCommand(uint8_t cmd, const uint8_t* payload, uint16_t len) {
    // Build header
    uint8_t hdr[10];
    hdr[0] = 0x00;           // direction = request
    hdr[1] = cmd;           // command
    hdr[2] = len & 0xFF;    // data length
    hdr[3] = (len>>8)&0xFF;
    uint16_t inv = ~len;
    hdr[4] = inv & 0xFF;     // size XOR
    hdr[5] = (inv>>8)&0xFF;
    uint32_t cks = 0;
    // For FLASH_DATA only: 8-bit seed=0xEF XOR each byte of actual data payload
    if (cmd == ESP_FLASH_DATA) {
      uint8_t c = 0xEF;
      for (uint16_t i = 16; i < len; i++) c ^= payload[i];
      cks = c; 
    }
    // pack checksum little‐endian into hdr[6..9]
    hdr[6] =  cks        &0xFF;
    hdr[7] = (cks >>  8)&0xFF;
    hdr[8] = (cks >> 16)&0xFF;
    hdr[9] = (cks >> 24)&0xFF;

    // SLIP frame the header+payload
    Serial2.write(SLIP_END);
    // write header
    for (uint8_t b: hdr) _slipWriteByte(b);
    // write payload
    for (uint16_t i = 0; i < len; i++) _slipWriteByte(payload[i]);
    Serial2.write(SLIP_END);
    return true;
  }

  // Read & parse one SLIP packet, expect a response to 'cmd'
  static bool receiveResponse(uint8_t cmd) {
    uint8_t buf[256];
    size_t  blen = 0;
    uint32_t start = millis();
    while (millis() - start < 3000) {
      if (_readSLIP(buf, blen, 300)) {
        if (blen < 8) continue;
        if (buf[0] != 0x01 || buf[1] != cmd) continue;
        uint16_t dlen = buf[2] | (buf[3]<<8);
        // status is last 2 or last 4 bytes of data
        size_t off = 8 + dlen;
        if (dlen >= 4) {
          // ROM loader uses 4-byte status
          uint8_t st = buf[off-4];
          return (st == 0);
        } else if (dlen >= 2) {
          uint8_t st = buf[off-2];
          return (st == 0);
        }
      }
    }
    return false;
  }

  // SLIP‐escape one byte
  static void _slipWriteByte(uint8_t c) {
    if (c == SLIP_END) {
      Serial2.write(SLIP_ESC);
      Serial2.write(SLIP_ESC_END);
    } else if (c == SLIP_ESC) {
      Serial2.write(SLIP_ESC);
      Serial2.write(SLIP_ESC_ESC);
    } else {
      Serial2.write(c);
    }
  }

  // Read one SLIP packet (strips framing & unescapes). Returns true on success.
  static bool _readSLIP(uint8_t* out, size_t &outLen, uint32_t timeout) {
    uint32_t t0 = millis();
    bool     started = false;
    size_t   idx = 0;
    while (millis() - t0 < timeout && idx < 256) {
      if (Serial2.available()) {
        uint8_t c = Serial2.read();
        if (!started) {
          if (c == SLIP_END) started = true;
        } else {
          if (c == SLIP_END) {
            outLen = idx;
            return true;
          } else if (c == SLIP_ESC) {
            // read one more
            while (!Serial2.available() && millis()-t0 < timeout);
            uint8_t n = Serial2.read();
            if (n == SLIP_ESC_END) out[idx++] = SLIP_END;
            else if (n == SLIP_ESC_ESC) out[idx++] = SLIP_ESC;
          } else {
            out[idx++] = c;
          }
        }
      }
    }
    return false;
  }
};

