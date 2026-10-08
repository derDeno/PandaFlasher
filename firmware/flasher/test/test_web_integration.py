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
    def test_preview_is_embedded_and_controls_have_handlers(self):
        page = (PROJECT / "web-preview.html").read_text(encoding="utf-8")
        embedded = (PROJECT / "src" / "webPage.h").read_text(encoding="utf-8")
        server = (PROJECT / "src" / "wifiWeb.h").read_text(encoding="utf-8")
        self.assertIn('R"WEBUI(' + page + ')WEBUI"', embedded)

        parser = PageParser()
        parser.feed(page)
        self.assertEqual(len(parser.ids), len(set(parser.ids)))
        for operation in parser.actions + ["flash", "reset", "delete"]:
            self.assertIn('op == "' + operation + '"', server)
        for endpoint in re.findall(r"api\(['\"](/api/[^'\"]+)", page):
            self.assertIn('server.on("' + endpoint + '"', server)


if __name__ == "__main__":
    unittest.main()
