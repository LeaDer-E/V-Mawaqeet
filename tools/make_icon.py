"""Render the current logo into assets/icon.ico (+ icon.png). Every size is drawn natively
(not downscaled), so the 16/24/32 px icons stay crisp. Run automatically by build_exe.py."""
import io, os, struct, sys
from pathlib import Path

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
root = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(root))

from PySide6.QtCore import QBuffer, QIODevice
from PySide6.QtWidgets import QApplication

app = QApplication([])
from app.theme import T
from app.ui import icons

T.set_mode("dark", "gold")                      # the exe icon always uses the default gold brand colours
SIZES = [256, 128, 64, 48, 32, 24, 16]


def png_bytes(size: int) -> bytes:
    pm = icons.logo_pixmap(size, 1.0)
    buf = QBuffer(); buf.open(QIODevice.WriteOnly); pm.save(buf, "PNG")
    return bytes(buf.data())


frames = [(s, png_bytes(s)) for s in SIZES]
out = bytearray(struct.pack("<HHH", 0, 1, len(frames)))
offset = 6 + 16 * len(frames)
body = bytearray()
for s, data in frames:
    out += struct.pack("<BBBBHHII", 0 if s >= 256 else s, 0 if s >= 256 else s, 0, 0, 1, 32, len(data), offset + len(body))
    body += data
(root / "assets" / "icon.ico").write_bytes(bytes(out) + bytes(body))
(root / "assets" / "icon.png").write_bytes(frames[0][1])
print("icon.ico written:", [s for s, _ in frames], len(out) + len(body), "bytes")
