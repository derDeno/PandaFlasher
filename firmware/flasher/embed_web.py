from pathlib import Path


def embed(project: Path) -> None:
    html = (project / "web-preview.html").read_text(encoding="utf-8")
    if ")WEBUI" in html:
        raise ValueError("web-preview.html conflicts with the C++ raw string delimiter")
    output = project / "src" / "webPage.h"
    source = (
        "#pragma once\n#include <pgmspace.h>\n"
        "// Generated from web-preview.html by embed_web.py.\n"
        'static const char WEB_PAGE[] PROGMEM = R"WEBUI(' + html + ')WEBUI";\n'
    )
    if not output.exists() or output.read_text(encoding="utf-8") != source:
        output.write_text(source, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    embed(Path(__file__).resolve().parent)
else:
    Import("env")
    embed(Path(env["PROJECT_DIR"]))
