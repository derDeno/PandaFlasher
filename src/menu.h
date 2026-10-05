
#define FF_FS_EXFAT 1
#define FF_USE_LFN 2

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SPIClass SPI2(FSPI);

// Menu handling
int currentMenu = 0;
int selectedOption = 0;
String sdFiles[20];
int fileCount = 0;
int selectedFile = 0;
bool filesLoaded = false;
uint32_t flashOffset = 0;
TargetInfo chipInfo;
String resultText;
File targetLog;
bool logFailed = false;
uint32_t lastLogFlushMs = 0;


// Target Serial Handling
static const uint8_t CHAR_W = 6;    // Default 5x7 font + 1px spacing
static const uint8_t CHAR_H = 8;
static const uint8_t COLS   = SCREEN_WIDTH / CHAR_W;   // ~21
static const uint8_t ROWS   = SCREEN_HEIGHT / CHAR_H;  // 8

char lines[ROWS][COLS + 1];         // ring buffer of lines (NUL-terminated)
uint8_t head = 0;                   // next line to write
char current[COLS + 1];
uint8_t curLen = 0;

unsigned long lastDrawMs = 0;

void clearBuffers() {
  for (uint8_t r = 0; r < ROWS; r++) { lines[r][0] = '\0'; }
  current[0] = '\0'; curLen = 0; head = 0;
}

void pushLine() {
  current[curLen] = '\0';
  strncpy(lines[head], current, COLS + 1);
  head = (head + 1) % ROWS;
  curLen = 0;
  current[0] = '\0';
}

void appendChar(char c) {
  if (c == '\r') return;                // ignore CR, handle LF only
  if (c == '\n') { pushLine(); return; }

  // Keep printable ASCII; drop others to avoid glyph issues
  if (c < 0x20 || c > 0x7E) return;

  if (curLen >= COLS) pushLine();       // wrap
  current[curLen++] = c;
}


// 'icon', 128x64px
const unsigned char logo[] PROGMEM = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x07, 0xf8, 0x00, 0x03, 0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x03, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x07, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x03, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x03, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xfc, 0x7f, 0xff, 0xfc, 0x07, 0xff, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xe0, 0x0f, 0xff, 0xf0, 0x01, 0xff, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0x80, 0x03, 0xff, 0xe0, 0x00, 0x7f, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x7f, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x3f, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x7e, 0x00, 0xe1, 0xff, 0xc0, 0x00, 0x1f, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x7c, 0x03, 0xf1, 0xff, 0x81, 0xe0, 0x0f, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x78, 0x03, 0xf0, 0xff, 0x83, 0xe0, 0x07, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x78, 0x03, 0xf0, 0xff, 0x83, 0xf0, 0x07, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x70, 0x03, 0xe0, 0xff, 0x83, 0xe0, 0x03, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x70, 0x00, 0x01, 0xff, 0x83, 0xe0, 0x03, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0x00, 0x03, 0xff, 0xe0, 0x00, 0x01, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0x00, 0x07, 0xc1, 0xf0, 0x00, 0x01, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0x00, 0x07, 0x80, 0x70, 0x00, 0x01, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x60, 0x00, 0x0f, 0x00, 0x38, 0x00, 0x01, 0x87, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xf8, 0x20, 0x00, 0x1e, 0x00, 0x3c, 0x00, 0x01, 0x07, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x20, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x01, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x30, 0x00, 0x7f, 0x00, 0x3f, 0x00, 0x01, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x30, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x02, 0x0f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfc, 0x18, 0x01, 0xff, 0x80, 0xff, 0x80, 0x06, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x1e, 0x03, 0xff, 0xf3, 0xff, 0xe0, 0x0c, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3c, 0x1f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xef, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xff, 0xf8, 0x7f, 0x8f, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0x83, 0xff, 0xfc, 0x1c, 0x0f, 0xff, 0xf0, 0x7f, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xfe, 0x00, 0x3f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0x80, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x3f, 0xff, 0xe3, 0xff, 0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0x03, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x07, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xff, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x0c, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void showSplashScreen() {
    display.clearDisplay();
    display.drawBitmap(0, 0, logo, 128, 64, SSD1306_WHITE);
    display.display();
    delay(4000);
}


/** MENU FUNCTIONS START */
void joystickTest() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Joystick Test:");

    if (digitalRead(JOY_UP) == LOW) display.println("UP");
    if (digitalRead(JOY_DOWN) == LOW) display.println("DOWN");
    if (digitalRead(JOY_LEFT) == LOW) display.println("LEFT");
    if (digitalRead(JOY_RIGHT) == LOW) display.println("RIGHT");
    if (digitalRead(JOY_CENTER) == LOW) display.println("CENTER");

    display.display();

    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = 0;
        delay(300);
    }
}

void showSoftwareVersion() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Software Version:");
    display.println(SOFTWARE_VERSION);
    display.display();

    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = 0;
        delay(300);
    }
}

void printDirectory(File dir) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break;

        String name = entry.name();
        if (name.startsWith("/")) name.remove(0, 1);
        String lower = name;
        lower.toLowerCase();
        if (!entry.isDirectory() && lower.endsWith(".bin") && fileCount < 20) {
            sdFiles[fileCount++] = name;
        }

        entry.close();
    }
}

void listSDCardFiles() {
    display.clearDisplay();
    display.setCursor(0, 0);

    if (!filesLoaded) {
        if (!SD.begin(SD_CS, SPI2)) {
            display.println("SD not found");
            display.display();
            delay(1000);
            currentMenu = 0;
            return;
        }
        fileCount = 0;
        File root = SD.open("/");
        printDirectory(root);
        root.close();
        selectedFile = 0;
        filesLoaded = true;
    }

    display.println("Select firmware:");
    if (!fileCount) display.println("No .bin files");
    int first = selectedFile < 5 ? 0 : selectedFile - 4;
    for (int i = first; i < fileCount && i < first + 5; i++) {
        display.print(i == selectedFile ? ">" : " ");
        display.println(sdFiles[i].substring(0, 20));
    }
    display.setCursor(0, 56);
    display.print("UP/DN Select  OK Open");
    display.display();

    if (fileCount && digitalRead(JOY_DOWN) == LOW) {
        selectedFile = (selectedFile + 1) % fileCount;
        delay(200);
    }
    if (fileCount && digitalRead(JOY_UP) == LOW) {
        selectedFile = (selectedFile + fileCount - 1) % fileCount;
        delay(200);
    }
    if (fileCount && digitalRead(JOY_CENTER) == LOW) {
        flashOffset = 0;
        currentMenu = 6;
        delay(200);
    }
    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = 0;
        filesLoaded = false;
        delay(300);
    }
}

void drawFlashProgress(uint32_t written, uint32_t total) {
    uint8_t percent = static_cast<uint8_t>((uint64_t)written * 100 / total);
    static uint8_t lastPercent = 255;
    if (percent == lastPercent && written != 0) return;
    lastPercent = percent;
    if (targetPort::flashLog && !targetPort::flashLog->printf("Progress: %u%%\n", percent)) {
        targetPort::flashLogFailed = true;
        targetPort::flashLog = nullptr;
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Flashing target");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.setCursor(0, 26);
    display.printf("%u%%", percent);
    display.drawRect(0, 40, 128, 10, SSD1306_WHITE);
    display.fillRect(2, 42, 124 * percent / 100, 6, SSD1306_WHITE);
    display.setCursor(0, 56);
    display.println("Keep connected");
    display.display();
}

void confirmFlash() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Flash this file?");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.println(flashOffset ? "Offset: 0x10000 APP" : "Offset: 0x0 MERGED");
    display.println("LEFT/RIGHT: offset");
    display.println("CENTER: flash");
    display.println("BACK: cancel");
    display.display();

    if (digitalRead(JOY_LEFT) == LOW || digitalRead(JOY_RIGHT) == LOW) {
        flashOffset = flashOffset ? 0 : 0x10000;
        delay(200);
    }
    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = 2;
        delay(200);
    }
    if (digitalRead(JOY_CENTER) != LOW) return;
    delay(200);

    File flashLog = SD.open("/flash-log.txt", FILE_APPEND);
    targetPort::flashLogFailed = !flashLog;
    targetPort::flashLog = flashLog ? &flashLog : nullptr;
    if (targetPort::flashLog && !flashLog.printf("\n--- Flash %s at 0x%lx ---\n", sdFiles[selectedFile].c_str(), static_cast<unsigned long>(flashOffset))) {
        targetPort::flashLogFailed = true;
        targetPort::flashLog = nullptr;
    }

    File file = SD.open("/" + sdFiles[selectedFile], FILE_READ);
    if (!file || !file.size() || file.read() != 0xE9) {
        resultText = "Invalid ESP .bin";
    } else if (!file.seek(0)) {
        resultText = "SD seek failed";
    } else {
        TargetFlasher flasher;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Connecting...");
        display.display();
        esp_loader_error_t err = flasher.connect();
        if (err == ESP_LOADER_SUCCESS) err = flasher.flash(file, flashOffset, drawFlashProgress);
        resultText = err == ESP_LOADER_SUCCESS ? "Flash verified!" : "Flash failed:\n" + String(targetErrorName(err));
        flasher.close();
    }
    if (targetPort::flashLog && !flashLog.println(resultText)) targetPort::flashLogFailed = true;
    targetPort::flashLog = nullptr;
    if (flashLog) flashLog.close();
    if (targetPort::flashLogFailed) resultText += "\nLog unavailable";
    file.close();
    currentMenu = 7;
}

void readChipDetails() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting...");
    display.display();
    TargetFlasher flasher;
    esp_loader_error_t err = flasher.connect(false);
    if (err == ESP_LOADER_SUCCESS) chipInfo = flasher.details();
    resultText = err == ESP_LOADER_SUCCESS ? "" : "Connect failed:\n" + String(targetErrorName(err));
    flasher.close();
    currentMenu = 5;
}

void showChipDetails() {
    display.clearDisplay();
    display.setCursor(0, 0);
    if (resultText.length()) {
        display.println(resultText);
    } else {
        display.println(targetChipName(chipInfo.chip));
        if (chipInfo.hasRevision) display.printf("Revision: %u.%u\n", chipInfo.revision / 100, chipInfo.revision % 100);
        else display.println("Revision: unknown");
        if (chipInfo.flashSize) display.printf("Flash: %lu MB\n", static_cast<unsigned long>(chipInfo.flashSize / 1048576));
        else display.println("Flash: unknown");
        if (chipInfo.hasMac) display.printf("MAC %02X:%02X:%02X:%02X:%02X:%02X\n", chipInfo.mac[0], chipInfo.mac[1], chipInfo.mac[2], chipInfo.mac[3], chipInfo.mac[4], chipInfo.mac[5]);
        if (chipInfo.hasSecurity) {
            display.printf("Secure boot: %s\n", chipInfo.secureBoot ? "ON" : "OFF");
            display.printf("Encryption: %s\n", chipInfo.flashEncryption ? "ON" : "OFF");
        }
    }
    display.setCursor(0, 56);
    display.println("BACK to menu");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 0; delay(200); }
}

void showFlashResult() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(resultText);
    display.println("BACK to files");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 2; delay(200); }
}
/** MENU FUNCTIONS END */

void setupMenu() {
    pinMode(JOY_UP, INPUT_PULLUP);
    pinMode(JOY_DOWN, INPUT_PULLUP);
    pinMode(JOY_LEFT, INPUT_PULLUP);
    pinMode(JOY_RIGHT, INPUT_PULLUP);
    pinMode(JOY_CENTER, INPUT_PULLUP);
    pinMode(BACK_BUTTON, INPUT_PULLUP);
    pinMode(SD_CS, OUTPUT);

    // OLED setup
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1315 OLED init failed!"));
        while (1);
    }

    showSplashScreen();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();

    uartSetup();
    clearBuffers();

    // SD card init
    SPI2.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI2)) {
        Serial.println("SD Card not found.");
    } else {
        Serial.println("SD Card initialized.");
    }
}

void showMainMenu() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Select an option:");
    static const char *options[] = {"Read Serial", "Flash from SD", "Chip details", "UART Test", "Software Version"};
    int first = selectedOption == 4 ? 1 : 0;
    for (int i = first; i < first + 4; ++i) {
        display.setCursor(10, 14 + (i - first) * 13);
        display.print(selectedOption == i ? "> " : "  ");
        display.println(options[i]);
    }
    display.display();

    if (digitalRead(JOY_DOWN) == LOW) {
        selectedOption = (selectedOption + 1) % 5;
        delay(200);
    }
    if (digitalRead(JOY_UP) == LOW) {
        selectedOption = (selectedOption + 4) % 5;  // Wrap around
        delay(200);
    }
    if (digitalRead(JOY_CENTER) == LOW) {
        currentMenu = selectedOption == 2 ? 4 : selectedOption == 3 ? 3 : selectedOption == 4 ? 8 : selectedOption + 1;
        if (currentMenu == 2) filesLoaded = false;
        if (currentMenu == 1) {
            clearBuffers();
            logFailed = false;
            if (SD.begin(SD_CS, SPI2)) targetLog = SD.open("/target-log.txt", FILE_APPEND);
            if (targetLog) {
                targetLog.println("\n--- target log session ---");
                lastLogFlushMs = millis();
            } else {
                logFailed = true;
            }
        }
        delay(200);
    }
}


void showUARTTest() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("UART Test:");
    display.display();

    if(digitalRead(JOY_CENTER) == LOW) {
        display.println("UART Test Started");
        display.display();
        PinControl::reset();
        delay(1000);
        display.println("UART Test Finished");
        display.display();
    }

    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = 0;
        delay(300);
    }
}


void drawScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    // Oldest line at top
    uint8_t idx = head;
    for (uint8_t r = 0; r < ROWS; r++) {
        uint8_t i = (idx + r) % ROWS;
        display.setCursor(0, r * CHAR_H);
        display.print(lines[i]);
    }

    // Draw the current (in-progress) line on the last row
    display.setCursor(0, (ROWS - 1) * CHAR_H);
    display.print(current);

    if (logFailed) {
        display.fillRect(92, 0, 36, 8, SSD1306_BLACK);
        display.setCursor(92, 0);
        display.print("SD ERR");
    }

    display.display();
}


void showTargetSerial() {
    while (TargetSerial.available()) {
        uint8_t buffer[128];
        size_t count = 0;
        while (count < sizeof(buffer) && TargetSerial.available()) {
            buffer[count] = TargetSerial.read();
            appendChar(static_cast<char>(buffer[count++]));
        }
        if (targetLog && targetLog.write(buffer, count) != count) {
            targetLog.close();
            logFailed = true;
        }
    }

    // Refresh ~20 Hz to reduce I2C traffic
    unsigned long now = millis();
    if (targetLog && now - lastLogFlushMs >= 1000) {
        targetLog.flush();
        lastLogFlushMs = now;
    }
    if (now - lastDrawMs >= 50) {
        drawScreen();
        lastDrawMs = now;
    }


    if (digitalRead(JOY_CENTER) == LOW) {
        TargetSerial.println("1");
    }


    if (digitalRead(BACK_BUTTON) == LOW) {
        if (targetLog) targetLog.close();
        currentMenu = 0;
        delay(300);
    }
}


void loopMenu() {
    switch (currentMenu) {
        case 0:
            showMainMenu();
            break;
        case 1:
            showTargetSerial();
            break;
        case 2:
            listSDCardFiles();
            break;
        case 3:
            showUARTTest();
            break;
        case 4:
            readChipDetails();
            break;
        case 5:
            showChipDetails();
            break;
        case 6:
            confirmFlash();
            break;
        case 7:
            showFlashResult();
            break;
        case 8:
            showSoftwareVersion();
            break;
    }
}
