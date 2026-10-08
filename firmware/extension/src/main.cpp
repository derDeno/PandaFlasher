#include <Arduino.h>
#include <esp_mac.h>
#include <esp_app_format.h>
#include <Update.h>

namespace {
constexpr int RX_PIN = 3;
constexpr int TX_PIN = 1;
constexpr int MUX_EN = 4;
constexpr int MUX_A2 = 5;
constexpr int MUX_A1 = 6;
constexpr int MUX_A0 = 7;
constexpr unsigned long IDLE_MS = 20;
constexpr char VERSION[] = "1.1.0";
constexpr size_t LINE_SIZE = 80;
constexpr size_t CHUNK_SIZE = 512;

HardwareSerial controlSerial(1);
char line[LINE_SIZE];
size_t length = 0;
bool collecting = false;
bool overflow = false;
uint8_t selected = 0;
unsigned long lastByte = 0;
uint32_t updateRemaining = 0;
size_t chunkLength = 0;
uint8_t chunk[CHUNK_SIZE];
unsigned long updateLastByte = 0;
bool updating = false;
bool firstChunk = false;

void receiveOnly() {
  controlSerial.end();
  pinMode(TX_PIN, INPUT);
  controlSerial.begin(115200, SERIAL_8N1, RX_PIN, -1);
}

void reply(const char *message) {
  // The mux must be off before this function drives PF-RX.
  controlSerial.end();
  controlSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  controlSerial.print(message);
  controlSerial.flush();
  receiveOnly();
}

void deselect() {
  digitalWrite(MUX_EN, LOW);
  selected = 0;
}

void select(uint8_t port) {
  digitalWrite(MUX_EN, LOW);
  const uint8_t address = 8 - port;
  digitalWrite(MUX_A2, (address >> 2) & 1);
  digitalWrite(MUX_A1, (address >> 1) & 1);
  digitalWrite(MUX_A0, address & 1);
  digitalWrite(MUX_EN, HIGH);
  selected = port;
}

void command() {
  line[length] = '\0';
  if (selected) {
    if (strcmp(line, "PFX1:DESELECT") == 0) {
      deselect();
      reply("OK DESELECT\n");
    }
    return; // Never drive PF-RX while a target is connected.
  }

  if (strcmp(line, "PFX1:ID") == 0) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char response[112];
    snprintf(response, sizeof(response),
             "OK ID HW=1.0 MCU=ESP32-C3-MINI-1U-N4 FW=%s MAC=%02X%02X%02X%02X%02X%02X PORTS=8\n",
             VERSION, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    reply(response);
  } else if (strcmp(line, "PFX1:DISCOVER") == 0) {
    reply("OK DISCOVER\n");
  } else if (strncmp(line, "PFX1:SELECT:", 12) == 0 && length == 13 &&
             line[12] >= '1' && line[12] <= '8') {
    const uint8_t port = line[12] - '0';
    char response[] = "OK SELECT 0\n";
    response[10] = line[12];
    reply(response);
    select(port);
  } else if (strcmp(line, "PFX1:DESELECT") == 0) {
    reply("OK DESELECT\n");
  } else if (strncmp(line, "PFX1:UPDATE:", 12) == 0) {
    unsigned long size = 0;
    char md5[33] = {};
    int end = 0;
    if (sscanf(line, "PFX1:UPDATE:%lu:%32[0-9a-f]%n", &size, md5, &end) != 2 ||
        end != static_cast<int>(length) || strlen(md5) != 32 || !size ||
        !Update.begin(size, U_FLASH) || !Update.setMD5(md5)) {
      Update.abort();
      reply("ERR UPDATE\n");
    } else {
      updateRemaining = size;
      chunkLength = 0;
      updateLastByte = millis();
      updating = true;
      firstChunk = true;
      reply("OK UPDATE\n");
      updateLastByte = millis();
    }
  } else if (strncmp(line, "PFX1:", 5) == 0) {
    reply("ERR COMMAND\n");
  }
}
} // namespace

void setup() {
  // Disable the decoder before configuring address outputs.
  digitalWrite(MUX_EN, LOW);
  pinMode(MUX_EN, OUTPUT);
  digitalWrite(MUX_A2, LOW);
  digitalWrite(MUX_A1, LOW);
  digitalWrite(MUX_A0, LOW);
  pinMode(MUX_A2, OUTPUT);
  pinMode(MUX_A1, OUTPUT);
  pinMode(MUX_A0, OUTPUT);
  pinMode(TX_PIN, INPUT);
  receiveOnly();
}

void loop() {
  if (updating && millis() - updateLastByte > 10000) {
    Update.abort();
    updating = false;
    reply("ERR TIMEOUT\n");
  }
  while (controlSerial.available()) {
    if (updating) {
      const uint8_t byte = controlSerial.read();
      chunk[chunkLength++] = byte;
      updateLastByte = millis();
      const size_t target = min<size_t>(CHUNK_SIZE, updateRemaining);
      if (chunkLength == target) {
        if (firstChunk) {
          esp_image_header_t header = {};
          uint32_t appMagic = 0;
          if (target >= sizeof(header) + sizeof(esp_image_segment_header_t) + sizeof(appMagic)) {
            memcpy(&header, chunk, sizeof(header));
            memcpy(&appMagic, chunk + sizeof(header) + sizeof(esp_image_segment_header_t), sizeof(appMagic));
          }
          firstChunk = false;
          if (header.magic != ESP_IMAGE_HEADER_MAGIC || header.chip_id != ESP_CHIP_ID_ESP32C3 ||
              appMagic != ESP_APP_DESC_MAGIC_WORD) {
            Update.abort();
            updating = false;
            chunkLength = 0;
            reply("ERR IMAGE\n");
            continue;
          }
        }
        const bool written = Update.write(chunk, target) == target;
        updateRemaining -= written ? target : 0;
        chunkLength = 0;
        if (!written) {
          Update.abort();
          updating = false;
          reply("ERR WRITE\n");
        } else if (!updateRemaining) {
          updating = false;
          if (Update.end()) {
            reply("OK DONE\n");
            delay(100);
            ESP.restart();
          } else reply("ERR VERIFY\n");
        } else {
          reply("OK CHUNK\n");
          updateLastByte = millis();
        }
      }
      continue;
    }
    const unsigned long now = millis();
    const char c = static_cast<char>(controlSerial.read());
    if (now - lastByte >= IDLE_MS) {
      length = 0;
      collecting = false;
      overflow = false;
    }
    const bool afterIdle = now - lastByte >= IDLE_MS;
    lastByte = now;

    if (selected && !collecting) {
      if (!afterIdle || c != 'P') continue;
      collecting = true;
    } else if (!selected && !collecting) {
      if (c != 'P') continue;
      collecting = true;
    }

    if (c == '\n') {
      if (!overflow) command();
      length = 0;
      collecting = false;
      overflow = false;
    } else if (length < LINE_SIZE - 1) {
      line[length++] = c;
      if (selected && strncmp(line, "PFX1:DESELECT", length) != 0) {
        collecting = false;
        length = 0;
      }
    } else {
      overflow = true;
    }
  }
}
