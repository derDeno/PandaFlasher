from html.parser import HTMLParser
from pathlib import Path
import re
import unittest


PROJECT = Path(__file__).resolve().parents[1]


class PageParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids = []
        self.actions = []

    def handle_starttag(self, tag, attributes):
        attrs = dict(attributes)
        if "id" in attrs:
            self.ids.append(attrs["id"])
        if "data-op" in attrs:
            self.actions.append(attrs["data-op"])


class WebIntegrationTest(unittest.TestCase):
    def test_littlefs_page_and_controls_have_handlers(self):
        page = (PROJECT / "data" / "index.html").read_text(encoding="utf-8")
        captive = (PROJECT / "data" / "captive.html").read_text(encoding="utf-8")
        server = (PROJECT / "src" / "wifiWeb.h").read_text(encoding="utf-8")
        platformio = (PROJECT / "platformio.ini").read_text(encoding="utf-8")
        partitions = (PROJECT / "partitions-8mb.csv").read_text(encoding="utf-8")
        self.assertIn("board_build.filesystem = littlefs", platformio)
        self.assertIn("app0,       app,  ota_0", partitions)
        self.assertIn("app1,       app,  ota_1", partitions)
        self.assertIn("otadata,    data, ota", partitions)
        self.assertIn("LittleFS.begin(false)", server)
        self.assertIn('LittleFS.open("/index.html", FILE_READ)', server)
        self.assertIn('LittleFS.open("/captive.html", FILE_READ)', server)
        self.assertIn('server.on("/save", HTTP_POST, saveWifi)', server)
        self.assertIn('server.on("/api/update/firmware", HTTP_POST, otaUploadDone, otaUploadChunk)', server)
        self.assertIn('server.on("/api/update/filesystem", HTTP_POST, otaUploadDone, otaUploadChunk)', server)
        self.assertIn('Update.begin(UPDATE_SIZE_UNKNOWN, otaUploadFilesystem ? U_SPIFFS : U_FLASH)', server)
        self.assertIn('Update.end(true)', server)
        self.assertIn('runExtensionUpdate(extensionWebProgress)', server)
        self.assertIn('name="token" value="{{TOKEN}}"', captive)

        parser = PageParser()
        parser.feed(page)
        self.assertEqual(len(parser.ids), len(set(parser.ids)))
        self.assertIn("update-dialog", parser.ids)
        self.assertIn("xhr.upload.addEventListener('progress'", page)
        self.assertIn("'/api/update/firmware'", page)
        self.assertIn("'/api/update/filesystem'", page)
        self.assertIn("async function updateExtension(file)", page)
        for form in ["firmware-update-form", "filesystem-update-form"]:
            self.assertIn(form, parser.ids)
        for operation in parser.actions + ["flash", "reset", "delete"]:
            self.assertIn('op == "' + operation + '"', server)
        for endpoint in re.findall(r"api\(['\"](/api/[^'\"]+)", page):
            self.assertIn('server.on("' + endpoint + '"', server)


if __name__ == "__main__":
    unittest.main()
