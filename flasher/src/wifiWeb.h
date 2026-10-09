#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <qrcode.h>
#include <ESPmDNS.h>
#include <mdns.h>
#include "webPage.h"

namespace WifiWeb {
WebServer server(80);
String apName, apPassword, token, mdnsName;
bool portal = false;
bool serverStarted = false;
bool webSerial = false;
bool mdnsStarted = false;
uint32_t mdnsLastAttemptMs = 0;
uint8_t serialPort = 0;
uint32_t serialLastPollMs = 0;
uint32_t wifiSwitchAt = 0;
File uploadFile;
String uploadName, uploadError;
bool uploadCreated = false;
size_t uploadBytes = 0;
bool flashStreaming = false;

void flashEvent(const String &event) {
    if (flashStreaming) server.sendContent(event + "\n");
}

void flashProgress(uint8_t port, uint8_t percent) {
    flashEvent("{\"type\":\"progress\",\"port\":" + String(port) + ",\"percent\":" + String(percent) + "}");
}

String escape(const String &value) {
    String out;
    for (char c : value) {
        if (c == '&') out += F("&amp;");
        else if (c == '<') out += F("&lt;");
        else if (c == '>') out += F("&gt;");
        else if (c == '"') out += F("&quot;");
        else if (c == '\'') out += F("&#39;");
        else out += c;
    }
    return out;
}

String jsonString(const String &value) {
    String out = "\"";
    for (char c : value) {
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (static_cast<uint8_t>(c) < 32) out += ' ';
        else out += c;
    }
    return out + '"';
}

bool validBinName(const String &name) {
    if (!name.length() || name.length() > 64 || name.indexOf('/') >= 0 ||
        name.indexOf('\\') >= 0 || name.indexOf("..") >= 0) return false;
    String lower = name;
    lower.toLowerCase();
    return lower.endsWith(".bin");
}

String duration(uint32_t ms) {
    const uint32_t minutes = ms / 60000;
    return String(minutes / 60) + " h " + String(minutes % 60) + " min";
}

bool postAllowed() {
    if (server.arg("token") == token) return true;
    server.send(403, "text/plain", "Reload the page and try again");
    return false;
}

String field(const char *name, const char *value) {
    return "<input type=hidden name='" + String(name) + "' value='" + escape(value) + "'>";
}

String pageStart(const char *title) {
    return "<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
           "<title>PandaFlasher</title><style>body{font:16px system-ui;max-width:48rem;margin:2rem auto;padding:0 1rem;background:#121821;color:#eee}"
           "a{color:#8cf}section{background:#202b39;padding:1rem;margin:1rem 0;border-radius:.7rem}"
           "input,select,button{font:inherit;padding:.45rem;margin:.25rem}button{cursor:pointer}pre{white-space:pre-wrap}</style>"
           "<h1>PandaFlasher</h1><h2>" + escape(title) + "</h2>";
}

void closeSerial() {
    webSerial = false;
    if (targetLog) targetLog.close();
    if (serialPort) { Extension::deselect(); serialPort = 0; }
}

void sendResult(const String &message) {
    server.send(200, "text/plain", message);
}

bool selectFile() {
    String name = server.arg("file");
    if (!validBinName(name)) return false;
    if (!SD.begin(SD_CS, SPI2)) return false;
    File file = SD.open("/" + name, FILE_READ);
    bool valid = file && !file.isDirectory() && file.size();
    if (file) file.close();
    if (valid) sdFiles[selectedFile = 0] = name;
    return valid;
}

void home() {
    server.send_P(200, "text/html", WEB_PAGE);
}

String macText(const uint8_t mac[6]) {
    char text[18];
    snprintf(text, sizeof(text), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return text;
}

void info() {
    if (Extension::active && !Extension::selected && !webSerial) Extension::detect();
    String out = "{\"token\":" + jsonString(token) +
        ",\"name\":" + jsonString(mdnsStarted ? mdnsName + ".local" : "Unavailable") +
        ",\"version\":" + jsonString(SOFTWARE_VERSION) +
        ",\"uptime\":" + jsonString(duration(millis())) +
        ",\"apSsid\":" + jsonString(apName) +
        ",\"ssid\":" + jsonString(portal ? apName : WiFi.SSID()) +
        ",\"ip\":" + jsonString((portal ? WiFi.softAPIP() : WiFi.localIP()).toString()) +
        ",\"mac\":" + jsonString(WiFi.macAddress()) +
        ",\"signal\":" + jsonString(portal ? "AP mode" : String(WiFi.RSSI()) + " dBm") +
        ",\"sdLogging\":" + String(sdLoggingEnabled ? "true" : "false") +
        ",\"extension\":";
    if (Extension::active) {
        char version[16] = {}, mac[13] = {};
        unsigned long uptime = 0;
        const char *fw = strstr(Extension::identity, " FW=");
        const char *up = strstr(Extension::identity, " UPTIME=");
        if (fw) sscanf(fw, " FW=%15s MAC=%12s", version, mac);
        if (up) sscanf(up, " UPTIME=%lu", &uptime);
        String address = "Unavailable";
        if (strlen(mac) == 12) {
            address = "";
            for (int i = 0; i < 12; i += 2) {
                if (i) address += ':';
                address += String(mac).substring(i, i + 2);
            }
        }
        out += "{\"version\":" + jsonString(version[0] ? version : "Unavailable") +
            ",\"uptime\":" + jsonString(up ? duration(uptime) : "Unavailable") +
            ",\"mac\":" + jsonString(address) + "}";
    } else out += "null";
    server.send(200, "application/json", out + "}");
}

void files() {
    if (!SD.begin(SD_CS, SPI2)) { server.send(503, "text/plain", "SD card unavailable"); return; }
    File root = SD.open("/");
    if (!root) { server.send(503, "text/plain", "SD card unavailable"); return; }
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "application/json", "");
    server.sendContent("[");
    bool first = true;
    while (File entry = root.openNextFile()) {
        String name = entry.name();
        if (name.startsWith("/")) name.remove(0, 1);
        if (!entry.isDirectory() && validBinName(name)) {
            server.sendContent(String(first ? "" : ",") + jsonString(name));
            first = false;
        }
        entry.close();
    }
    root.close();
    server.sendContent("]");
    server.sendContent("");
}

void identify() {
    if (webSerial) closeSerial();
    String out = "[";
    bool first = true;
    if (Extension::active && !Extension::discover()) {
        server.send(503, "text/plain", "PandaExtension link unavailable");
        return;
    }
    const uint8_t firstPort = Extension::active ? 1 : 0;
    const uint8_t lastPort = Extension::active ? 8 : 0;
    bool linkFailed = false;
    for (uint8_t port = firstPort; port <= lastPort; ++port) {
        if (Extension::active && !Extension::select(port)) { linkFailed = true; break; }
        TargetFlasher flasher;
        esp_loader_error_t err = flasher.connect(false);
        TargetInfo target;
        if (err == ESP_LOADER_SUCCESS) target = flasher.details();
        flasher.close();
        if (Extension::active && !Extension::deselect()) { linkFailed = true; break; }
        if (err != ESP_LOADER_SUCCESS) continue;
        out += String(first ? "" : ",") + "{\"port\":" + String(port) +
            ",\"name\":" + jsonString(port ? "Port " + String(port) : "Direct target") +
            ",\"chip\":" + jsonString(targetChipName(target.chip)) +
            ",\"mac\":" + jsonString(target.hasMac ? macText(target.mac) : "Unavailable") + "}";
        first = false;
    }
    if (linkFailed) { server.send(503, "text/plain", "PandaExtension link failed during scan"); return; }
    server.send(200, "application/json", out + "]");
}

bool selectedPorts(uint16_t &mask) {
    String text = server.arg("ports");
    if (!text.length() || text.length() > 3) return false;
    unsigned value = 0;
    for (char c : text) {
        if (c < '0' || c > '9') return false;
        value = value * 10 + c - '0';
    }
    if (!value || value > 0x1fe || (Extension::active ? (value & ~0x1fe) : value != 1)) return false;
    mask = value;
    return true;
}

void uploadChunk() {
    HTTPUpload &part = server.upload();
    if (part.status == UPLOAD_FILE_START) {
        uploadName = part.filename;
        uploadError = "";
        uploadCreated = false;
        uploadBytes = 0;
        if (server.header("X-Panda-Token") != token) uploadError = "Reload the page and try again";
        else if (!validBinName(uploadName)) uploadError = "Select a .bin file with a simple name";
        else if (!SD.begin(SD_CS, SPI2)) uploadError = "SD card unavailable";
        else if (SD.exists("/" + uploadName)) uploadError = "A file with that name already exists";
        else {
            uploadFile = SD.open("/" + uploadName, FILE_WRITE);
            if (!uploadFile) uploadError = "Could not create file";
            else uploadCreated = true;
        }
    } else if (part.status == UPLOAD_FILE_WRITE && uploadFile && !uploadError.length()) {
        if (uploadFile.write(part.buf, part.currentSize) != part.currentSize) uploadError = "SD write failed";
        else uploadBytes += part.currentSize;
    } else if (part.status == UPLOAD_FILE_END || part.status == UPLOAD_FILE_ABORTED) {
        if (part.status == UPLOAD_FILE_ABORTED) uploadError = "Upload cancelled";
        if (!uploadBytes && !uploadError.length()) uploadError = "Empty file";
        if (uploadFile) { uploadFile.close(); uploadFile = File(); }
        if (uploadError.length() && uploadCreated) SD.remove("/" + uploadName);
    }
}

void uploadDone() {
    if (server.header("X-Panda-Token") != token) { server.send(403, "text/plain", "Reload the page and try again"); return; }
    if (!uploadName.length()) uploadError = "No file received";
    if (uploadFile) { uploadFile.close(); uploadFile = File(); }
    server.send(uploadError.length() ? 400 : 200, "text/plain", uploadError.length() ? uploadError : "Uploaded " + uploadName);
    uploadName = "";
    uploadError = "";
    uploadCreated = false;
}

void action() {
    if (!postAllowed()) return;
    String op = server.arg("op");
    if (webSerial && op != "uart") closeSerial();
    if (op == "wifi") { startSetup(); sendResult("Connect to the setup AP shown on the OLED."); return; }
    if (op == "logging") {
        const String enabled = server.arg("enabled");
        if (enabled != "0" && enabled != "1") { server.send(400, "text/plain", "Invalid SD logging setting"); return; }
        saveSdLogging(enabled == "1");
        sendResult(sdLoggingEnabled ? "SD log files enabled" : "SD log files disabled");
        return;
    }
    if (op == "ap") {
        Preferences prefs;
        prefs.begin("wifi", false);
        prefs.putBool("forceAp", true);
        prefs.end();
        sendResult("Restarting in AP mode");
        restartAtMs = millis() + 1000;
        return;
    }
    if (op == "reboot") { sendResult("Restarting PandaFlasher"); restartAtMs = millis() + 1000; return; }
    if (op == "extension-restart") {
        if (!Extension::restart()) { server.send(503, "text/plain", "PandaExtension unavailable"); return; }
        Extension::active = false;
        delay(500);
        Extension::detect();
        sendResult("PandaExtension restarted");
        return;
    }
    if (op == "uart") { TargetSerial.println("1"); sendResult("Sent 1 to target UART"); return; }
    if (op == "delete") {
        String name = server.arg("file");
        if (!validBinName(name) || !SD.begin(SD_CS, SPI2) || !SD.remove("/" + name)) {
            server.send(400, "text/plain", "Could not delete firmware file"); return;
        }
        sendResult("Deleted " + name);
        return;
    }
    if (op == "reset") {
        uint16_t mask;
        if (!selectedPorts(mask)) { server.send(400, "text/plain", "Invalid selected ports"); return; }
        String result;
        for (uint8_t port = Extension::active ? 1 : 0; port <= (Extension::active ? 8 : 0); ++port) {
            if (!(mask & (1 << port))) continue;
            bool okay = !Extension::active || Extension::select(port);
            if (okay) PinControl::reset();
            if (Extension::active && okay) okay = Extension::deselect();
            result += (port ? "Port " + String(port) : "Direct target") + (okay ? ": reset\n" : ": failed\n");
            if (!okay) break;
        }
        sendResult(result);
        return;
    }
    if (op != "flash" && op != "self" && op != "extension") { server.send(400, "text/plain", "Unknown action"); return; }
    if (op == "extension" && !Extension::active) { server.send(400, "text/plain", "Extension unavailable"); return; }
    if (!selectFile()) { server.send(400, "text/plain", "Select a valid SD .bin file"); return; }
    if (op == "flash") {
        uint16_t mask;
        if (!selectedPorts(mask)) { server.send(400, "text/plain", "Invalid selected ports"); return; }
        File image = SD.open("/" + sdFiles[selectedFile], FILE_READ);
        const bool merged = image && image.size() > 0x10000 && image.read() == 0xE9 &&
            image.seek(0x8000) && image.read() == 0xAA && image.read() == 0x50 &&
            image.seek(0x10000) && image.read() == 0xE9;
        if (image) image.close();
        if (!merged) { server.send(400, "text/plain", "Use a merged image with a partition table at 0x8000 and app at 0x10000"); return; }
        flashOffset = 0;
        flashEraseAll = true;
        server.setContentLength(CONTENT_LENGTH_UNKNOWN);
        server.send(200, "application/x-ndjson", "");
        flashStreaming = true;
        flashEvent("{\"type\":\"log\",\"text\":\"Starting flash of " + jsonString(sdFiles[selectedFile]) + "\"}");
        for (uint8_t port = Extension::active ? 1 : 0; port <= (Extension::active ? 8 : 0); ++port) {
            if (!(mask & (1 << port))) continue;
            flashPort = port;
            flashEvent("{\"type\":\"progress\",\"port\":" + String(port) + ",\"percent\":0}");
            flashEvent("{\"type\":\"log\",\"text\":" + jsonString((port ? "Port " + String(port) : "Direct target") + ": flashing") + "}");
            runFlash();
            flashEvent("{\"type\":\"result\",\"port\":" + String(port) + ",\"text\":" + jsonString(resultText) + "}");
            flashEvent("{\"type\":\"log\",\"text\":" + jsonString((port ? "Port " + String(port) : "Direct target") + ": " + resultText) + "}");
        }
        flashEraseAll = false;
        flashStreaming = false;
        server.sendContent("");
        return;
    } else if (op == "self") runSelfUpdate();
    else runExtensionUpdate();
    sendResult(resultText);
}

void serialPage() {
    if (webSerial) closeSerial();
    int port = server.arg("port").toInt();
    if (Extension::active && (port < 1 || port > 8 || !Extension::select(port))) {
        server.send(400, "text/plain", "Select a reachable extension port");
        return;
    }
    serialPort = Extension::active ? port : 0;
    webSerial = true;
    serialLastPollMs = millis();
    clearBuffers();
    logFailed = false;
    if (sdLoggingEnabled && SD.begin(SD_CS, SPI2)) targetLog = SD.open("/target-log.txt", FILE_APPEND);
    if (targetLog) targetLog.println("\n--- web target log session ---");
    else logFailed = sdLoggingEnabled;
    String html = pageStart("Target serial") + "<pre id=log></pre><form method=post action='/action'>" +
        field("token", token.c_str()) + "<button name=op value=uart>Send 1</button></form><a href='/serial-stop'>Stop and return</a><script>";
    html += "setInterval(async()=>{let r=await fetch('/serial-data');if(r.ok)document.getElementById('log').textContent=await r.text()},1000)</script>";
    server.send(200, "text/html", html);
}

void stopSerial() {
    closeSerial();
    server.sendHeader("Location", "/");
    server.send(303);
}

void serialData() {
    serialLastPollMs = millis();
    String output;
    for (uint8_t i = 0; i < ROWS; ++i) output += String(lines[(head + i) % ROWS]) + "\n";
    output += current;
    server.send(200, "text/plain", output);
}

void saveWifi() {
    if (!portal || !postAllowed()) return;
    String ssid = server.arg("ssid"), pass = server.arg("password");
    if (!ssid.length() || ssid.length() > 32 || pass.length() > 63 || (pass.length() && pass.length() < 8)) { server.send(400, "text/plain", "Invalid Wi-Fi credentials"); return; }
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.putBool("forceAp", false);
    prefs.end();
    server.send(200, "text/html", pageStart("Saved") + "<p>Connecting. The OLED will show the device address.</p>");
    WiFi.begin(ssid.c_str(), pass.c_str());
    wifiSwitchAt = millis() + 1500;
}

void saveApName() {
    if (!postAllowed()) return;
    String name = server.arg("apSsid");
    if (!name.length() || name.length() > 32) { server.send(400, "text/plain", "AP name must be 1 to 32 characters"); return; }
    if (portal && !WiFi.softAP(name.c_str(), apPassword.c_str())) { server.send(503, "text/plain", "Could not rename the setup access point"); return; }
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("apName", name);
    prefs.end();
    apName = name;
    sendResult(portal ? "Access point renamed. Reconnect using the new name." : "Access point name saved.");
}

void setupPage() {
    if (!portal) { home(); return; }
    server.send(200, "text/html", pageStart("Wi-Fi setup") +
        "<form method=post action='/save'>" + field("token", token.c_str()) +
        "<label>Network name <input name=ssid required maxlength=32></label><br>"
        "<label>Password <input name=password type=password maxlength=63></label><br>"
        "<button>Connect</button></form>");
}

void drawQr(esp_qrcode_handle_t qr) {
    const int size = esp_qrcode_get_size(qr);
    const int scale = size + 4 <= 32 ? 2 : 1;
    display.fillRect(0, 0, (size + 4) * scale, (size + 4) * scale, SSD1306_WHITE);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x)
            if (esp_qrcode_get_module(qr, x, y))
                display.fillRect((x + 2) * scale, (y + 2) * scale, scale, scale, SSD1306_BLACK);
}

void drawSetup() {
    display.clearDisplay();
    display.setCursor(0, 0);
    if (!portal && WiFi.status() == WL_CONNECTED) {
        display.println("Wi-Fi connected");
        display.println(WiFi.localIP());
    } else {
        String qrText = "WIFI:T:WPA;S:" + apName + ";P:" + apPassword + ";;";
        esp_qrcode_config_t qr = {drawQr, 5, ESP_QRCODE_ECC_LOW};
        if (esp_qrcode_generate(&qr, qrText.c_str()) != ESP_OK) display.println("QR unavailable");
        display.setCursor(0, 43);
        display.println(apName);
        display.setCursor(0, 54);
        display.println(apPassword);
    }
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 16; delay(200); }
}

void drawStatus() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(WiFi.status() == WL_CONNECTED ? "Wi-Fi connected" : "Wi-Fi offline");
    if (WiFi.status() == WL_CONNECTED) {
        display.println(WiFi.localIP());
    }
    display.println("BACK to Settings");
    display.display();
    if (digitalRead(BACK_BUTTON) == LOW) { currentMenu = 16; delay(200); }
}

void startSetup() {
    if (mdnsStarted) { MDNS.end(); mdnsStarted = false; mdnsName = ""; }
    mdnsLastAttemptMs = 0;
    portal = true;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(apName.c_str(), apPassword.c_str());
    if (!serverStarted) { server.begin(); serverStarted = true; }
}

void begin() {
    uint64_t mac = ESP.getEfuseMac();
    char suffix[5];
    snprintf(suffix, sizeof(suffix), "%04lX", static_cast<unsigned long>(mac & 0xffff));
    apName = "PandaFlasher-" + String(suffix);
    Preferences prefs;
    prefs.begin("settings", true);
    sdLoggingEnabled = prefs.getBool("sdLog", true);
    prefs.end();
    prefs.begin("wifi", false);
    apName = prefs.getString("apName", apName);
    if (!apName.length() || apName.length() > 32) apName = "PandaFlasher-" + String(suffix);
    apPassword = prefs.getString("apkey", "");
    if (apPassword.length() < 12) {
        char key[17];
        snprintf(key, sizeof(key), "%08lX%08lX", static_cast<unsigned long>(esp_random()), static_cast<unsigned long>(esp_random()));
        apPassword = key;
        prefs.putString("apkey", apPassword);
    }
    String ssid = prefs.getString("ssid", ""), pass = prefs.getString("pass", "");
    bool forceAp = prefs.getBool("forceAp", false);
    prefs.end();
    char csrf[9];
    snprintf(csrf, sizeof(csrf), "%08lX", static_cast<unsigned long>(esp_random()));
    token = csrf;
    if (ssid.length() && !forceAp) { WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(), pass.c_str()); }
    const char *headers[] = {"X-Panda-Token"};
    server.collectHeaders(headers, 1);
    server.on("/", HTTP_GET, setupPage);
    server.on("/save", HTTP_POST, saveWifi);
    server.on("/api/ap-name", HTTP_POST, saveApName);
    server.on("/action", HTTP_POST, action);
    server.on("/api/info", HTTP_GET, info);
    server.on("/api/files", HTTP_GET, files);
    server.on("/api/identify", HTTP_GET, identify);
    server.on("/api/action", HTTP_POST, action);
    server.on("/api/upload", HTTP_POST, uploadDone, uploadChunk);
    server.on("/serial", HTTP_GET, serialPage);
    server.on("/serial-data", HTTP_GET, serialData);
    server.on("/serial-stop", HTTP_GET, stopSerial);
    if (forceAp || !ssid.length()) startSetup();
    else { server.begin(); serverStarted = true; }
}

void startMdns() {
    mdnsLastAttemptMs = millis();
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    if (mac.length() < 4) return;
    String fallback = "pandaflasher-" + mac.substring(mac.length() - 4);
    fallback.toLowerCase();
    if (!MDNS.begin(fallback)) return;
    mdnsName = fallback;
    mdnsStarted = true;
    esp_ip4_addr_t address = {};
    if (mdns_query_a("pandaflasher", 1500, &address) == ESP_ERR_NOT_FOUND &&
        mdns_hostname_set("pandaflasher") == ESP_OK) mdnsName = "pandaflasher";
}

void loop() {
    server.handleClient();
    if (mdnsStarted && WiFi.status() != WL_CONNECTED) {
        MDNS.end();
        mdnsStarted = false;
        mdnsName = "";
    }
    if (!mdnsStarted && !portal && WiFi.status() == WL_CONNECTED &&
        (!mdnsLastAttemptMs || millis() - mdnsLastAttemptMs >= 10000)) startMdns();
    if (webSerial && millis() - serialLastPollMs > 5000) closeSerial();
    if (webSerial && currentMenu != 1) {
        uint8_t buffer[128];
        size_t count = 0;
        while (count < sizeof(buffer) && TargetSerial.available()) {
            buffer[count] = TargetSerial.read();
            appendChar(static_cast<char>(buffer[count++]));
        }
        if (targetLog && count && targetLog.write(buffer, count) != count) { targetLog.close(); logFailed = true; }
        if (targetLog && count) targetLog.flush();
    }
    if (portal && wifiSwitchAt && static_cast<int32_t>(millis() - wifiSwitchAt) >= 0 && WiFi.status() == WL_CONNECTED) {
        portal = false;
        wifiSwitchAt = 0;
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
    }
}
}
