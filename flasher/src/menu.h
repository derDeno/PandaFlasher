
#define FF_FS_EXFAT 1
#define FF_USE_LFN 2

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>
#include <Preferences.h>

namespace WifiWeb { void startSetup(); void drawSetup(); void drawStatus(); void begin(); void loop(); void flashProgress(uint8_t port, uint8_t percent); }


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
bool flashEraseAll = false;
uint8_t flashPort = 0; // 0 = all responding ports, 1..8 = one port
uint8_t progressPort = 0;
bool selectingExtensionImage = false;
bool selectingSelfImage = false;
uint8_t extensionOption = 0;
TargetInfo chipInfo;
String resultText;
File targetLog;
bool logFailed = false;
bool sdLoggingEnabled = true;
uint32_t lastLogFlushMs = 0;
uint32_t restartAtMs = 0;


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

void showExtensionMenu() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Extension");
    display.println(extensionOption == 0 ? "> Status" : "  Status");
    display.println(extensionOption == 1 ? "> Update firmware" : "  Update firmware");
    display.setCursor(0, 56);
    display.println("UP/DN  CENTER  BACK");
    display.display();
    if (digitalRead(JOY_UP) == LOW || digitalRead(JOY_DOWN) == LOW) { extensionOption ^= 1; delay(200); }
    if (digitalRead(JOY_CENTER) == LOW) {
        if (extensionOption == 0) currentMenu = 11;
        else { selectingExtensionImage = true; filesLoaded = false; currentMenu = 2; }
        delay(200);
    }
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 0; delay(200); }
}

void showExtensionStatus() {
    char hw[8] = {}, mcu[32] = {}, fw[16] = {}, mac[13] = {};
    unsigned ports = 0;
    const bool valid = sscanf(Extension::identity,
        "OK ID HW=%7s MCU=%31s FW=%15s MAC=%12s PORTS=%u",
        hw, mcu, fw, mac, &ports) == 5;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Extension: ACTIVE");
    if (valid) {
        display.printf("HW %s  Ports %u\n", hw, ports);
        display.printf("FW %s\n", fw);
        display.println(mcu);
        display.printf("MAC %s\n", mac);
    } else display.println("ID unavailable");
    display.setCursor(0, 56);
    display.println("BACK to Extension");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 10; delay(200); }
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

    display.println(selectingExtensionImage ? "Select extension .bin:" : selectingSelfImage ? "Select Panda .bin:" : "Select firmware:");
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
        flashPort = 0;
        currentMenu = selectingExtensionImage ? 12 : selectingSelfImage ? 14 : Extension::active ? 9 : 6;
        delay(200);
    }
    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = selectingExtensionImage ? 10 : selectingSelfImage ? 16 : 0;
        selectingExtensionImage = false;
        selectingSelfImage = false;
        filesLoaded = false;
        delay(300);
    }
}

void drawExtensionUpdateProgress(uint32_t sent, uint32_t total) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Updating extension");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.printf("%lu%%\n", static_cast<unsigned long>((uint64_t)sent * 100 / total));
    display.drawRect(0, 40, 128, 10, SSD1306_WHITE);
    display.fillRect(2, 42, 124 * sent / total, 6, SSD1306_WHITE);
    display.display();
}

void runExtensionUpdate();

void confirmExtensionUpdate() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Update extension?");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.println("C3 app .bin only");
    display.println("Keep power connected");
    display.println("CENTER send  BACK");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 2; delay(200); return; }
    if (digitalRead(JOY_CENTER) != LOW) return;
    delay(200);
    runExtensionUpdate();
    currentMenu = 13;
}

void runExtensionUpdate() {
    File file = SD.open("/" + sdFiles[selectedFile], FILE_READ);
    resultText = file ? Extension::update(file, drawExtensionUpdateProgress) : "SD open failed";
    if (file) file.close();
    if (resultText == "Update sent!") {
        delay(500);
        Extension::detect();
        resultText = Extension::active ? "Extension updated!" : "Sent; restart flasher";
    }
    selectingExtensionImage = false;
}

void showExtensionUpdateResult() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(resultText);
    display.println("BACK to menu");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = Extension::active ? 10 : 0; delay(200); }
}

void drawSelfUpdateProgress(uint32_t written, uint32_t total) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Updating Panda");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.printf("%lu%%\n", static_cast<unsigned long>((uint64_t)written * 100 / total));
    display.drawRect(0, 40, 128, 10, SSD1306_WHITE);
    display.fillRect(2, 42, 124 * written / total, 6, SSD1306_WHITE);
    display.display();
}

void runSelfUpdate();

void confirmSelfUpdate() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Update PandaFlasher?");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.println("S3 app .bin only");
    display.println("Keep power connected");
    display.println("CENTER start  BACK");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 2; delay(200); return; }
    if (digitalRead(JOY_CENTER) != LOW) return;
    delay(200);
    runSelfUpdate();
    currentMenu = 15;
}

void runSelfUpdate() {
    File file = SD.open("/" + sdFiles[selectedFile], FILE_READ);
    resultText = file ? SelfUpdate::install(file, drawSelfUpdateProgress) : "SD open failed";
    if (file) file.close();
    selectingSelfImage = false;
    if (resultText == "Update verified") {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Update verified");
        display.println("Restarting...");
        display.display();
        restartAtMs = millis() + 1500;
    }
}

void showSelfUpdateResult() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(resultText);
    display.println("BACK to menu");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 16; delay(200); }
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
    WifiWeb::flashProgress(progressPort, percent);
    display.clearDisplay();
    display.setCursor(0, 0);
    if (progressPort) display.printf("Flashing port %u\n", progressPort);
    else display.println("Flashing target");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.setCursor(0, 26);
    display.printf("%u%%", percent);
    display.drawRect(0, 40, 128, 10, SSD1306_WHITE);
    display.fillRect(2, 42, 124 * percent / 100, 6, SSD1306_WHITE);
    display.setCursor(0, 56);
    display.println("Keep connected");
    display.display();
}

void chooseFlashPort() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Extension active");
    display.println("Flash destination:");
    if (flashPort) display.printf("Port %u\n", flashPort);
    else display.println("All responding ports");
    display.println("LEFT/RIGHT choose");
    display.println("CENTER continue");
    display.println("BACK files");
    display.display();
    if (digitalRead(JOY_RIGHT) == LOW) { flashPort = (flashPort + 1) % 9; delay(200); }
    if (digitalRead(JOY_LEFT) == LOW) { flashPort = (flashPort + 8) % 9; delay(200); }
    if (digitalRead(JOY_CENTER) == LOW) { currentMenu = 6; delay(200); }
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 2; delay(200); }
}

void runFlash();

void confirmFlash() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Flash this file?");
    display.println(sdFiles[selectedFile].substring(0, 21));
    display.println(flashOffset ? "Offset: 0x10000 APP" : "Offset: 0x0 MERGED");
    if (Extension::active) display.println(flashPort ? "Port: " + String(flashPort) : "All responding ports");
    display.println("LEFT/RIGHT: offset");
    display.println("CENTER: flash");
    display.println("BACK: cancel");
    display.display();

    if (digitalRead(JOY_LEFT) == LOW || digitalRead(JOY_RIGHT) == LOW) {
        flashOffset = flashOffset ? 0 : 0x10000;
        delay(200);
    }
    if (digitalRead(BACK_BUTTON) == LOW) {
        currentMenu = Extension::active ? 9 : 2;
        delay(200);
    }
    if (digitalRead(JOY_CENTER) != LOW) return;
    delay(200);
    runFlash();
    currentMenu = 7;
}

void runFlash() {
    File flashLog;
    if (sdLoggingEnabled) flashLog = SD.open("/flash-log.txt", FILE_APPEND);
    targetPort::flashLogFailed = sdLoggingEnabled && !flashLog;
    targetPort::flashLog = flashLog ? &flashLog : nullptr;
    if (targetPort::flashLog && !(Extension::active
        ? flashLog.printf("\n--- Flash %s at 0x%lx, port %u ---\n", sdFiles[selectedFile].c_str(), static_cast<unsigned long>(flashOffset), flashPort)
        : flashLog.printf("\n--- Flash %s at 0x%lx ---\n", sdFiles[selectedFile].c_str(), static_cast<unsigned long>(flashOffset)))) {
        targetPort::flashLogFailed = true;
        targetPort::flashLog = nullptr;
    }

    File file = SD.open("/" + sdFiles[selectedFile], FILE_READ);
    if (!file || !file.size() || file.read() != 0xE9) {
        resultText = "Invalid ESP .bin";
    } else if (!file.seek(0)) {
        resultText = "SD seek failed";
    } else if (Extension::active) {
        unsigned passed = 0, failed = 0, absent = 0;
        bool linkFailed = false;
        const uint8_t first = flashPort ? flashPort : 1;
        const uint8_t last = flashPort ? flashPort : 8;
        if (!flashPort && !Extension::discover()) linkFailed = true;
        for (uint8_t port = first; port <= last && !linkFailed; ++port) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.printf("Checking port %u\n", port);
            display.display();
            if (!Extension::select(port)) { linkFailed = true; break; }
            TargetFlasher flasher;
            esp_loader_error_t err = flasher.connect();
            if (err == ESP_LOADER_SUCCESS) {
                if (!file.seek(0)) err = ESP_LOADER_ERROR_FAIL;
                else { progressPort = port; err = flasher.flash(file, flashOffset, drawFlashProgress, flashEraseAll); }
            }
            flasher.close();
            if (targetPort::flashLog && !flashLog.printf("Port %u: %s\n", port, targetErrorName(err))) {
                targetPort::flashLogFailed = true;
                targetPort::flashLog = nullptr;
            }
            if (err == ESP_LOADER_SUCCESS) ++passed;
            else if (!flashPort && err == ESP_LOADER_ERROR_TIMEOUT) ++absent;
            else ++failed;
            if (!Extension::deselect()) { linkFailed = true; break; }
        }
        progressPort = 0;
        if (flashPort) resultText = linkFailed ? "Extension link failed" : passed ? "Port " + String(flashPort) + " verified!" : "Port " + String(flashPort) + " failed";
        else resultText = String(passed) + " verified\n" + String(failed) + " failed\n" + String(absent) + " no response" + (linkFailed ? "\nExtension link failed" : "");
    } else {
        TargetFlasher flasher;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Connecting...");
        display.display();
        esp_loader_error_t err = flasher.connect();
        if (err == ESP_LOADER_SUCCESS) err = flasher.flash(file, flashOffset, drawFlashProgress, flashEraseAll);
        resultText = err == ESP_LOADER_SUCCESS ? "Flash verified!" : "Flash failed:\n" + String(targetErrorName(err));
        flasher.close();
    }
    if (targetPort::flashLog && !flashLog.println(resultText)) targetPort::flashLogFailed = true;
    targetPort::flashLog = nullptr;
    if (flashLog) flashLog.close();
    if (targetPort::flashLogFailed) resultText += "\nLog unavailable";
    file.close();
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
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Checking extension...");
    display.display();
    Extension::detect();
    clearBuffers();

    // SD card init
    SPI2.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI2)) {
        Serial.println("SD Card not found.");
    } else {
        Serial.println("SD Card initialized.");
    }
    WifiWeb::begin();
}

uint8_t settingsOption = 0;

void saveSdLogging(bool enabled) {
    sdLoggingEnabled = enabled;
    Preferences prefs;
    prefs.begin("settings", false);
    prefs.putBool("sdLog", enabled);
    prefs.end();
    if (!enabled && targetLog) targetLog.close();
}

void showSettings() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Settings");
    const char *items[] = {"WiFi setup", "WiFi status", "Update Panda", "Restart Panda", sdLoggingEnabled ? "SD logs: On" : "SD logs: Off"};
    for (int i = 0; i < 5; ++i) {
        display.print(i == settingsOption ? "> " : "  ");
        display.println(items[i]);
    }
    display.setCursor(0, 56);
    display.println("UP/DN CENTER BACK");
    display.display();
    if (digitalRead(JOY_DOWN) == LOW) { settingsOption = (settingsOption + 1) % 5; delay(200); }
    if (digitalRead(JOY_UP) == LOW) { settingsOption = (settingsOption + 4) % 5; delay(200); }
    if (digitalRead(JOY_CENTER) == LOW) {
        if (settingsOption == 0) { WifiWeb::startSetup(); currentMenu = 17; }
        else if (settingsOption == 1) currentMenu = 19;
        else if (settingsOption == 2) { selectingSelfImage = true; filesLoaded = false; currentMenu = 2; }
        else if (settingsOption == 3) currentMenu = 18;
        else saveSdLogging(!sdLoggingEnabled);
        delay(200);
    }
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 0; delay(200); }
}

void confirmRestart() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Restart PandaFlasher?");
    display.println("CENTER confirm");
    display.println("BACK cancel");
    display.display();
    if (digitalRead(JOY_CENTER) == LOW) ESP.restart();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 16; delay(200); }
}

void showMainMenu() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(Extension::active ? "EXT ACTIVE  Select:" : "Select an option:");
    static const char *options[] = {"Read Serial", "Flash from SD", "Chip details", "UART Test", "Software Version", "Settings", "Extension"};
    const int optionCount = Extension::active ? 7 : 6;
    int first = selectedOption >= 4 ? selectedOption - 3 : 0;
    for (int i = first; i < first + 4; ++i) {
        display.setCursor(10, 14 + (i - first) * 13);
        display.print(selectedOption == i ? "> " : "  ");
        display.println(options[i]);
    }
    display.display();

    if (digitalRead(JOY_DOWN) == LOW) {
        selectedOption = (selectedOption + 1) % optionCount;
        delay(200);
    }
    if (digitalRead(JOY_UP) == LOW) {
        selectedOption = (selectedOption + optionCount - 1) % optionCount;  // Wrap around
        delay(200);
    }
    if (digitalRead(JOY_CENTER) == LOW) {
        currentMenu = selectedOption == 6 ? 10 : selectedOption == 5 ? 16 : selectedOption == 2 ? 4 : selectedOption == 3 ? 3 : selectedOption == 4 ? 8 : selectedOption + 1;
        if (currentMenu == 2) filesLoaded = false;
        if (currentMenu == 1) {
            clearBuffers();
            logFailed = false;
            if (sdLoggingEnabled && SD.begin(SD_CS, SPI2)) targetLog = SD.open("/target-log.txt", FILE_APPEND);
            if (targetLog) {
                targetLog.println("\n--- target log session ---");
                lastLogFlushMs = millis();
            } else if (sdLoggingEnabled) {
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
        case 9:
            chooseFlashPort();
            break;
        case 10:
            showExtensionMenu();
            break;
        case 11:
            showExtensionStatus();
            break;
        case 12:
            confirmExtensionUpdate();
            break;
        case 13:
            showExtensionUpdateResult();
            break;
        case 14:
            confirmSelfUpdate();
            break;
        case 15:
            showSelfUpdateResult();
            break;
        case 16:
            showSettings();
            break;
        case 17:
            WifiWeb::drawSetup();
            break;
        case 18:
            confirmRestart();
            break;
        case 19:
            WifiWeb::drawStatus();
            break;
    }
}
