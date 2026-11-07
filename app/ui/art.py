"""Procedural illustrations for the six prayers, painted with QPainter.

Each scene is layered (sky -> celestial body -> clouds -> far skyline -> mosque
-> ground -> palms -> haze) so it reads as depth rather than flat shapes.
Scenes are rendered once per size into a QPixmap and cached.
"""
from __future__ import annotations
import math, random
from dataclasses import dataclass
from PySide6.QtCore import Qt, QPointF, QRectF
from PySide6.QtGui import (QImage, QBrush, QColor, QLinearGradient, QPainter, QPainterPath, QPen, QPixmap,
                           QRadialGradient)

C = QColor


@dataclass
class Scene:
    sky: list                 # [(pos, color)]
    body: str                 # mosque main color
    shade: str                # mosque shade color
    dome: str
    dome_shade: str
    far: str                  # far skyline color
    ground: tuple             # (top, bottom)
    hero: tuple               # (c1, c2) used by the hero card gradient
    sil: bool = False         # pure silhouette?
    windows: str | None = None


SCENES: dict[str, Scene] = {
    "fajr": Scene([(0, "#0A1038"), (.42, "#262C74"), (.70, "#5F4D9C"), (.88, "#C9809F"), (1, "#F2B49C")],
                  "#0E1128", "#0A0C1E", "#0E1128", "#0A0C1E", "#2A2A5E", ("#0B0D20", "#06070F"),
                  ("#232B6B", "#6B4F9E"), True, None),
    "sunrise": Scene([(0, "#3C4C92"), (.40, "#C37C8C"), (.70, "#F0A468"), (.90, "#F8CB7C"), (1, "#FFE6A6")],
                     "#2B1D26", "#1C121A", "#2B1D26", "#1C121A", "#B5707A", ("#24161C", "#120B10"),
                     ("#9A5B6E", "#F0A468"), True, None),
    "dhuhr": Scene([(0, "#1B78CC"), (.55, "#5FB1EA"), (1, "#C4E6F7")],
                   "#F4EAD3", "#CDBB96", "#EADFC4", "#B7C4C9", "#A9CFE8", ("#E6D8B8", "#B9A57C"),
                   ("#1B78CC", "#7CC0EE"), False, "#6E8FA3"),
    "asr": Scene([(0, "#5F7DA6"), (.45, "#CFA777"), (.80, "#EFCB8E"), (1, "#F8E0AE")],
                 "#E9C998", "#A97F4A", "#DDB46F", "#9A7440", "#C9A576", ("#D2B27F", "#8E6D3D"),
                 ("#B0864F", "#E9C98F"), False, "#7A5A32"),
    "maghrib": Scene([(0, "#271650"), (.32, "#7F2A68"), (.60, "#D9524F"), (.82, "#F59553"), (1, "#FDC587")],
                     "#1B0F1E", "#12081A", "#1B0F1E", "#12081A", "#6A2A52", ("#170B18", "#0B050E"),
                     ("#8A2D6B", "#F0704F"), True, None),
    "isha": Scene([(0, "#040920"), (.55, "#0D1C52"), (.85, "#1A3478"), (1, "#27508F")],
                  "#070B1B", "#04060F", "#070B1B", "#04060F", "#14224E", ("#060A18", "#03050C"),
                  ("#0E1D50", "#27508F"), True, "#FFC866"),
}


def hero_colors(key: str) -> tuple[QColor, QColor]:
    s = SCENES[key]
    return C(s.hero[0]), C(s.hero[1])


_U = 170.0      # current "unit" (card width, or banner height) used to scale stars/clouds


def _rg(cx, cy, r, stops) -> QRadialGradient:
    g = QRadialGradient(cx, cy, r)
    for pos, col in stops:
        g.setColorAt(pos, col)
    return g


def _glow(p: QPainter, cx, cy, r, color: QColor, strength=0.9):
    c0 = QColor(color); c0.setAlphaF(strength)
    c1 = QColor(color); c1.setAlphaF(strength * 0.25)
    c2 = QColor(color); c2.setAlphaF(0)
    p.setPen(Qt.NoPen)
    p.setBrush(QBrush(_rg(cx, cy, r, [(0, c0), (.35, c1), (1, c2)])))
    p.drawEllipse(QPointF(cx, cy), r, r)


def _stars(p: QPainter, w, h, n, seed, ymax=0.6, bright=1.0):
    rnd = random.Random(seed)
    p.setPen(Qt.NoPen)
    for _ in range(n):
        x, y = rnd.random() * w, rnd.random() * h * ymax
        fade = 1 - (y / (h * ymax)) ** 1.6
        r = rnd.choice([0.6, 0.7, 0.9, 1.1, 1.5]) * (_U / 170)
        a = max(0.0, min(1.0, (0.35 + rnd.random() * 0.65) * fade * bright))
        col = C(255, 250, 235, int(255 * a))
        p.setBrush(col)
        p.drawEllipse(QPointF(x, y), r, r)
        if r > 1.3 * (_U / 170) and a > 0.5:         # twinkle cross
            p.setPen(QPen(C(255, 250, 235, int(120 * a)), 0.6 * (_U / 170)))
            p.drawLine(QPointF(x - r * 3, y), QPointF(x + r * 3, y))
            p.drawLine(QPointF(x, y - r * 3), QPointF(x, y + r * 3))
            p.setPen(Qt.NoPen)


def _crescent(p: QPainter, cx, cy, r, color: QColor, thickness=0.78, tilt=-20):
    path = QPainterPath()
    path.addEllipse(QPointF(cx, cy), r, r)
    cut = QPainterPath()
    off = r * (1 - thickness) * 1.9
    cut.addEllipse(QPointF(cx + off, cy - off * 0.35), r * 0.9, r * 0.9)
    p.save(); p.translate(cx, cy); p.rotate(tilt); p.translate(-cx, -cy)
    p.setPen(Qt.NoPen); p.setBrush(color)
    p.drawPath(path.subtracted(cut))
    p.restore()


def _clouds(p: QPainter, w, h, color: QColor, items):
    p.setPen(Qt.NoPen)
    for (x, y, rx, ry, a) in items:
        for dx, dy, k in ((0, 0, 1), (-.55, .1, .7), (.6, .12, .75), (.15, -.2, .65)):
            col0 = QColor(color); col0.setAlphaF(a)
            col1 = QColor(color); col1.setAlphaF(0)
            cx, cy = x * w + dx * rx * _U, (y + dy * ry) * h
            g = _rg(cx, cy, rx * _U * k, [(0, col0), (1, col1)])
            p.setBrush(QBrush(g))
            p.save(); p.translate(cx, cy); p.scale(1, ry / rx * (_U / h)); p.translate(-cx, -cy)
            p.drawEllipse(QPointF(cx, cy), rx * _U * k, rx * _U * k)
            p.restore()


def _rays(p: QPainter, cx, cy, length, color: QColor, n=14, spread=math.pi, start=math.pi, alpha=0.10):
    p.setPen(Qt.NoPen)
    for i in range(n):
        a = start + spread * (i + 0.5) / n
        wdt = spread / n * 0.42
        path = QPainterPath(QPointF(cx, cy))
        path.lineTo(cx + length * math.cos(a - wdt), cy + length * math.sin(a - wdt))
        path.lineTo(cx + length * math.cos(a + wdt), cy + length * math.sin(a + wdt))
        path.closeSubpath()
        g = QLinearGradient(cx, cy, cx + length * math.cos(a), cy + length * math.sin(a))
        c0 = QColor(color); c0.setAlphaF(alpha); c1 = QColor(color); c1.setAlphaF(0)
        g.setColorAt(0, c0); g.setColorAt(1, c1)
        p.setBrush(QBrush(g)); p.drawPath(path)


def _onion(cx, base_y, r, height, pointy=0.35) -> QPainterPath:
    path = QPainterPath(QPointF(cx - r, base_y))
    path.cubicTo(cx - r * 1.14, base_y - height * 0.50, cx - r * pointy, base_y - height * 0.78, cx, base_y - height)
    path.cubicTo(cx + r * pointy, base_y - height * 0.78, cx + r * 1.14, base_y - height * 0.50, cx + r, base_y)
    path.closeSubpath()
    return path


def _arch(cx, base_y, w, h) -> QPainterPath:
    path = QPainterPath(QPointF(cx - w / 2, base_y))
    path.lineTo(cx - w / 2, base_y - h * 0.55)
    path.cubicTo(cx - w / 2, base_y - h * 0.85, cx - w * 0.12, base_y - h * 0.92, cx, base_y - h)
    path.cubicTo(cx + w * 0.12, base_y - h * 0.92, cx + w / 2, base_y - h * 0.85, cx + w / 2, base_y - h * 0.55)
    path.lineTo(cx + w / 2, base_y)
    path.closeSubpath()
    return path


def _shaded(p, rect: QRectF, light: QColor, dark: QColor, horizontal=True):
    g = QLinearGradient(rect.left(), 0, rect.right(), 0) if horizontal else QLinearGradient(0, rect.top(), 0, rect.bottom())
    g.setColorAt(0, light); g.setColorAt(.55, light); g.setColorAt(1, dark)
    return QBrush(g)


def _finial(p: QPainter, x, y, u, color: QColor, gold: QColor | None = None):
    p.setPen(QPen(gold or color, max(1.0, u * 0.006)))
    p.drawLine(QPointF(x, y), QPointF(x, y - u * 0.045))
    p.setPen(Qt.NoPen); p.setBrush(gold or color)
    p.drawEllipse(QPointF(x, y - u * 0.05), u * 0.011, u * 0.011)


def _minaret(p: QPainter, x, by, u, sc: Scene, lit_right: bool):
    body, shade, dome = C(sc.body), C(sc.shade), C(sc.dome)
    sw = u * 0.040
    top1 = by - u * 0.40
    r1 = QRectF(x - sw / 2, top1, sw, by - top1)
    p.setPen(Qt.NoPen)
    p.setBrush(body if sc.sil else _shaded(p, r1, body, shade))
    p.drawRect(r1)
    bw = u * 0.082
    p.setBrush(body if sc.sil else shade.lighter(112))
    p.drawRect(QRectF(x - bw / 2, top1 - u * 0.018, bw, u * 0.018))          # balcony
    p.drawRect(QRectF(x - bw * 0.42, top1 - u * 0.034, bw * 0.84, u * 0.016))
    top2 = top1 - u * 0.13
    r2 = QRectF(x - sw * 0.38, top2, sw * 0.76, top1 - u * 0.034 - top2)
    p.setBrush(body if sc.sil else _shaded(p, r2, body, shade))
    p.drawRect(r2)
    p.drawRect(QRectF(x - sw * 0.62, top2 - u * 0.008, sw * 1.24, u * 0.012))
    cap = _onion(x, top2 - u * 0.008, sw * 0.62, u * 0.105, 0.2)
    p.setBrush(C(sc.dome) if sc.sil else dome)
    p.drawPath(cap)
    _finial(p, x, top2 - u * 0.113, u, C(sc.dome) if sc.sil else C("#D6A84A"))
    if sc.windows:                                                              # slit windows
        p.setBrush(C(sc.windows))
        for k in range(3):
            p.drawRoundedRect(QRectF(x - u * 0.004, top1 + u * 0.05 + k * u * 0.095, u * 0.008, u * 0.036), u * 0.004, u * 0.004)


def mosque(p: QPainter, cx, by, u, sc: Scene):
    """Main mosque: portal hall, drum, central onion dome, two side domes, two minarets."""
    body, shade, dome, dshade = C(sc.body), C(sc.shade), C(sc.dome), C(sc.dome_shade)
    p.setPen(Qt.NoPen)
    for sx in (-1, 1):
        _minaret(p, cx + sx * u * 0.395, by, u, sc, lit_right=sx > 0)
    # hall
    hall = QRectF(cx - u * 0.31, by - u * 0.25, u * 0.62, u * 0.25)
    p.setBrush(body if sc.sil else _shaded(p, hall, body, shade))
    p.drawRect(hall)
    p.setBrush(shade if not sc.sil else body)
    p.drawRect(QRectF(hall.left() - u * 0.01, hall.top() - u * 0.012, hall.width() + u * 0.02, u * 0.016))
    # side domes (behind drum)
    for sx in (-1, 1):
        x = cx + sx * u * 0.215
        path = _onion(x, hall.top() - u * 0.004, u * 0.088, u * 0.13, 0.3)
        p.setBrush(dome if sc.sil else _shaded(p, path.boundingRect(), dome, dshade))
        p.drawPath(path)
        _finial(p, x, hall.top() - u * 0.134, u, dome if sc.sil else C("#D6A84A"))
    # drum
    drum = QRectF(cx - u * 0.155, hall.top() - u * 0.075, u * 0.31, u * 0.075)
    p.setBrush(body if sc.sil else _shaded(p, drum, body, shade))
    p.drawRect(drum)
    # central dome
    path = _onion(cx, drum.top() + u * 0.002, u * 0.175, u * 0.285, 0.30)
    p.setBrush(dome if sc.sil else _shaded(p, path.boundingRect(), dome, dshade))
    p.drawPath(path)
    if not sc.sil:                                     # soft highlight rib on the dome
        hl = QColor("#FFFFFF"); hl.setAlphaF(0.22)
        p.setPen(QPen(hl, u * 0.006)); p.setBrush(Qt.NoBrush)
        p.drawArc(QRectF(cx - u * 0.12, drum.top() - u * 0.22, u * 0.14, u * 0.2), 100 * 16, 60 * 16)
        p.setPen(Qt.NoPen)
    _finial(p, cx, drum.top() - u * 0.285, u, dome if sc.sil else C("#D6A84A"))
    # openings
    win = C(sc.windows) if sc.windows else (shade.darker(150) if not sc.sil else C(sc.shade))
    p.setBrush(win)
    p.drawPath(_arch(cx, by, u * 0.13, u * 0.19))                                  # portal
    for sx in (-1, 1):
        for k in (1, 2):
            p.drawPath(_arch(cx + sx * (u * 0.12 + k * u * 0.075), by - u * 0.03, u * 0.036, u * 0.115))
    for k in range(-2, 3):
        p.drawRoundedRect(QRectF(cx + k * u * 0.058 - u * 0.011, drum.top() + u * 0.02, u * 0.022, u * 0.038), u * 0.011, u * 0.011)
    if sc.windows:                                    # inner portal glow
        g = _rg(cx, by - u * 0.08, u * 0.12, [(0, C(255, 220, 140, 150)), (1, C(255, 200, 100, 0))])
        p.setBrush(QBrush(g)); p.drawEllipse(QPointF(cx, by - u * 0.08), u * 0.12, u * 0.12)


def _far_skyline(p: QPainter, w, by, u, color: QColor, seed: int):
    rnd = random.Random(seed)
    p.setPen(Qt.NoPen); p.setBrush(color)
    x = -u * 0.05
    while x < w + u * 0.05:
        bw = u * rnd.uniform(0.10, 0.20)
        bh = u * rnd.uniform(0.05, 0.12)
        p.drawRect(QRectF(x, by - bh, bw, bh + u))
        r = rnd.random()
        if r < 0.42:
            p.drawPath(_onion(x + bw / 2, by - bh, bw * 0.38, bw * 0.55, 0.3))
        elif r < 0.68:
            mw = u * 0.022
            p.drawRect(QRectF(x + bw * 0.5 - mw / 2, by - bh - u * rnd.uniform(0.12, 0.22), mw, u * 0.3))
            p.drawPath(_onion(x + bw * 0.5, by - bh - u * 0.18, mw * 0.8, mw * 2.4, 0.2))
        x += bw * rnd.uniform(0.75, 1.0)


def _palm(p: QPainter, x, by, h, color: QColor, lean=0.1):
    p.setPen(Qt.NoPen); p.setBrush(color)
    tw = h * 0.035
    tx, ty = x + lean * h, by - h
    trunk = QPainterPath(QPointF(x - tw, by))
    trunk.quadTo(x + lean * h * 0.2, by - h * 0.5, tx - tw * 0.4, ty)
    trunk.lineTo(tx + tw * 0.4, ty)
    trunk.quadTo(x + lean * h * 0.2 + tw, by - h * 0.5, x + tw, by)
    trunk.closeSubpath()
    p.drawPath(trunk)
    for k, ang in enumerate((-160, -135, -108, -80, -52, -25, 0, 20, -190)):
        a = math.radians(ang)
        L = h * (0.42 if k % 2 else 0.50)
        ex, ey = tx + L * math.cos(a), ty + L * math.sin(a) + L * 0.42
        mx, my = tx + L * 0.55 * math.cos(a), ty + L * 0.55 * math.sin(a) - L * 0.12
        leaf = QPainterPath(QPointF(tx, ty))
        leaf.quadTo(mx, my - h * 0.025, ex, ey)
        leaf.quadTo(mx, my + h * 0.03, tx, ty)
        p.drawPath(leaf)


def _birds(p: QPainter, pts, color: QColor, size: float):
    p.setBrush(Qt.NoBrush); p.setPen(QPen(color, max(1.0, size * 0.18), Qt.SolidLine, Qt.RoundCap))
    for x, y, k in pts:
        s = size * k
        path = QPainterPath(QPointF(x - s, y - s * 0.2))
        path.quadTo(x - s * 0.5, y - s * 0.7, x, y)
        path.quadTo(x + s * 0.5, y - s * 0.7, x + s, y - s * 0.2)
        p.drawPath(path)


def render(key: str, w: int, h: int, dpr: float = 2.0, unit: float | None = None, cx: float | None = None) -> QPixmap:
    """Paint a scene. unit scales the mosque (defaults to width); cx is the mosque centre (defaults to w/2)."""
    global _U
    sc = SCENES[key]
    pm = QPixmap(int(w * dpr), int(h * dpr))
    pm.setDevicePixelRatio(dpr)
    pm.fill(Qt.transparent)
    p = QPainter(pm)
    p.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
    u = float(unit or w)
    _U = u
    mx = cx if cx is not None else w / 2
    by = h * 0.775                       # mosque base line
    # --- sky
    g = QLinearGradient(0, 0, 0, by + u * 0.05)
    for pos, col in sc.sky:
        g.setColorAt(pos, C(col))
    p.fillRect(QRectF(0, 0, w, h), QBrush(g))
    night = key in ("fajr", "isha")
    # --- celestial + atmosphere per prayer
    if key == "fajr":
        _stars(p, w, h, 70, 11, 0.55, 0.9)
        _glow(p, w * 0.5, by, u * 0.9, C("#F4A98F"), 0.55)
        _glow(p, w * 0.70, h * 0.17, u * 0.34, C("#CFC8FF"), 0.45)
        _crescent(p, w * 0.70, h * 0.17, u * 0.075, C("#F7F0D6"), 0.72, -28)
        _clouds(p, w, h, C("#D890B0"), [(0.25, 0.46, 0.33, 0.035, 0.30), (0.78, 0.55, 0.30, 0.03, 0.26)])
    elif key == "sunrise":
        sx, sy = w * 0.75, by - u * 0.31
        _glow(p, sx, sy, u * 1.15, C("#FFD27A"), 0.95)
        _rays(p, sx, sy, h * 0.9, C("#FFE9B0"), 16, math.pi, math.pi, 0.17)
        gg = _rg(sx, sy, u * 0.17, [(0, C("#FFF8DC")), (.7, C("#FFE19A")), (1, C("#FFC768"))])
        p.setPen(Qt.NoPen); p.setBrush(QBrush(gg)); p.drawEllipse(QPointF(sx, sy), u * 0.17, u * 0.17)
        _stars(p, w, h, 14, 5, 0.25, 0.5)
        _clouds(p, w, h, C("#FFD7A8"), [(0.22, 0.40, 0.34, 0.03, 0.40), (0.80, 0.30, 0.30, 0.028, 0.34), (0.55, 0.52, 0.36, 0.03, 0.30)])
    elif key == "dhuhr":
        sx, sy = w * 0.66, h * 0.15
        _glow(p, sx, sy, u * 0.95, C("#FFFFFF"), 0.85)
        _rays(p, sx, sy, h * 1.1, C("#FFFFFF"), 18, math.pi * 0.95, math.pi * 0.02, 0.10)
        p.setPen(Qt.NoPen); p.setBrush(C("#FFFDF0")); p.drawEllipse(QPointF(sx, sy), u * 0.095, u * 0.095)
        _clouds(p, w, h, C("#FFFFFF"), [(0.22, 0.30, 0.30, 0.03, 0.85), (0.80, 0.44, 0.34, 0.034, 0.80), (0.38, 0.55, 0.32, 0.03, 0.70)])
    elif key == "asr":
        sx, sy = w * 0.80, h * 0.36
        _glow(p, sx, sy, u * 1.1, C("#FFD58A"), 0.9)
        _rays(p, sx, sy, h * 0.9, C("#FFE2A8"), 14, math.pi * 0.9, math.pi * 0.55, 0.12)
        p.setPen(Qt.NoPen); p.setBrush(C("#FFF0C0")); p.drawEllipse(QPointF(sx, sy), u * 0.10, u * 0.10)
        _clouds(p, w, h, C("#FBE3B6"), [(0.28, 0.32, 0.34, 0.03, 0.55), (0.62, 0.48, 0.36, 0.03, 0.45)])
    elif key == "maghrib":
        sx, sy = w * 0.26, by - u * 0.30
        _glow(p, sx, sy, u * 1.25, C("#FF8A4B"), 0.95)
        p.setPen(Qt.NoPen)
        gg = _rg(sx, sy, u * 0.21, [(0, C("#FFE3A3")), (.6, C("#FF9A55")), (1, C("#F0603F"))])
        p.setBrush(QBrush(gg)); p.drawEllipse(QPointF(sx, sy), u * 0.21, u * 0.21)
        _clouds(p, w, h, C("#FF9A7A"), [(0.30, 0.36, 0.36, 0.03, 0.50), (0.78, 0.30, 0.30, 0.026, 0.42), (0.6, 0.50, 0.36, 0.03, 0.40)])
        _stars(p, w, h, 18, 3, 0.22, 0.6)
    elif key == "isha":
        _stars(p, w, h, 120, 29, 0.66, 1.0)
        _glow(p, w * 0.30, h * 0.20, u * 0.60, C("#9DB4FF"), 0.40)
        _glow(p, w * 0.30, h * 0.20, u * 0.20, C("#FFF4D0"), 0.55)
        _crescent(p, w * 0.30, h * 0.20, u * 0.115, C("#FFF6DA"), 0.74, -26)
        _glow(p, w * 0.5, by, u * 0.9, C("#3A6AA8"), 0.35)
    # --- far skyline (atmospheric perspective)
    _far_skyline(p, w, by + u * 0.01, u, C(sc.far), hash(key) % 997)
    # --- mosque
    mosque(p, mx, by, u, sc)
    # --- ground
    gr = QLinearGradient(0, by, 0, h)
    gr.setColorAt(0, C(sc.ground[0])); gr.setColorAt(1, C(sc.ground[1]))
    p.setPen(Qt.NoPen); p.setBrush(QBrush(gr))
    p.drawRect(QRectF(0, by, w, h - by + 1))
    if key == "asr":                                             # long cast shadow
        sh = QPainterPath(QPointF(w * 0.18, by))
        sh.lineTo(w * 0.82, by); sh.lineTo(w * 0.40, by + (h - by) * 0.72); sh.lineTo(-w * 0.15, by + (h - by) * 0.72); sh.closeSubpath()
        p.setBrush(C(60, 36, 12, 70)); p.drawPath(sh)
    if key == "dhuhr":
        p.setBrush(C(255, 255, 255, 60)); p.drawRect(QRectF(0, by, w, u * 0.012))
    # --- reflection of the mosque on the courtyard
    img = pm.toImage()
    sh = int(min((h - by) * dpr * 0.9, by * dpr))
    src = img.copy(0, int(by * dpr) - sh, int(w * dpr), sh).mirrored(False, True)
    mask = QPainter(src); mask.setCompositionMode(QPainter.CompositionMode_DestinationIn)
    mg = QLinearGradient(0, 0, 0, sh); mg.setColorAt(0, C(0, 0, 0, 90 if night else 60)); mg.setColorAt(1, C(0, 0, 0, 0))
    mask.fillRect(src.rect(), QBrush(mg)); mask.end()
    p.drawImage(QRectF(0, by, w, sh / dpr), src)
    # --- palms / birds
    pal = C(sc.ground[1])
    if key in ("fajr", "maghrib", "isha", "sunrise"):
        _palm(p, -w * 0.01, by + u * 0.015, u * 0.55, pal, 0.13)
        _palm(p, w * 1.01, by + u * 0.015, u * 0.46, pal, -0.12)
    if key == "asr":
        _palm(p, w * 1.01, by + u * 0.02, u * 0.50, C("#6E5128"), -0.12)
    if key == "maghrib":
        _birds(p, [(w * .62, h * .24, 1), (w * .70, h * .20, .8), (w * .56, h * .19, .7), (w * .76, h * .27, .6), (w * .66, h * .30, .5)],
               C("#2A1228"), u * 0.035)
    if key == "dhuhr":
        _birds(p, [(w * .30, h * .20, 1), (w * .37, h * .17, .7)], C("#4B6A80"), u * 0.03)
    # --- horizon haze + vignette for depth
    hz = QLinearGradient(0, by - u * 0.22, 0, by + u * 0.02)
    hc = C(sc.sky[-1][1]); hc0 = QColor(hc); hc0.setAlphaF(0.0); hc1 = QColor(hc); hc1.setAlphaF(0.30 if not night else 0.10)
    hz.setColorAt(0, hc0); hz.setColorAt(1, hc1)
    p.setPen(Qt.NoPen); p.setBrush(QBrush(hz)); p.drawRect(QRectF(0, by - u * 0.22, w, u * 0.24))
    vg = _rg(w / 2, h * 0.45, h * 0.85, [(0, C(0, 0, 0, 0)), (.65, C(0, 0, 0, 0)), (1, C(0, 0, 0, 70 if night else 38))])
    p.setBrush(QBrush(vg)); p.drawRect(QRectF(0, 0, w, h))
    p.end()
    return pm
