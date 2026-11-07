"""Vector icons drawn with QPainter (crisp at any DPI, theme-tintable, no font glyphs)."""
from __future__ import annotations
import math
from PySide6.QtCore import Qt, QPointF, QRectF
from PySide6.QtGui import QColor, QPainter, QPainterPath, QPen, QPixmap, QIcon, QBrush


def _pen(c: QColor, w: float) -> QPen:
    p = QPen(c, w)
    p.setCapStyle(Qt.RoundCap); p.setJoinStyle(Qt.RoundJoin)
    return p


def draw(p: QPainter, name: str, r: QRectF, color: QColor, w: float | None = None) -> None:
    """Draw icon `name` inside rect r."""
    p.save()
    p.setRenderHint(QPainter.Antialiasing)
    s = min(r.width(), r.height())
    p.translate(r.center().x() - s / 2, r.center().y() - s / 2)
    p.scale(s / 24, s / 24)
    p.setPen(_pen(color, w if w else 1.9))
    p.setBrush(Qt.NoBrush)
    f = _ICONS.get(name)
    if f:
        f(p, color)
    p.restore()


def pixmap(name: str, size: int, color: QColor, dpr: float = 2.0, w: float | None = None) -> QPixmap:
    pm = QPixmap(int(size * dpr), int(size * dpr))
    pm.setDevicePixelRatio(dpr)
    pm.fill(Qt.transparent)
    p = QPainter(pm)
    draw(p, name, QRectF(0, 0, size, size), color, w)
    p.end()
    return pm


def icon(name: str, size: int, color: QColor) -> QIcon:
    return QIcon(pixmap(name, size, color))


# ---- individual icons on a 24x24 grid -------------------------------------
def _home(p, c):
    path = QPainterPath()
    path.moveTo(3.5, 11); path.lineTo(12, 3.8); path.lineTo(20.5, 11)
    p.drawPath(path)
    b = QPainterPath()
    b.moveTo(5.5, 9.6); b.lineTo(5.5, 19.5); b.quadTo(5.5, 20.5, 6.5, 20.5); b.lineTo(17.5, 20.5)
    b.quadTo(18.5, 20.5, 18.5, 19.5); b.lineTo(18.5, 9.6)
    p.drawPath(b)
    p.drawPath(_poly([(10, 20.5), (10, 15), (14, 15), (14, 20.5)]))


def _poly(pts):
    path = QPainterPath(QPointF(*pts[0]))
    for x, y in pts[1:]:
        path.lineTo(x, y)
    return path


def _gear(p, c):
    cx = cy = 12
    path = QPainterPath()
    n = 8
    for i in range(n * 2):
        ang = math.pi * i / n - math.pi / (2 * n) * 0
        rr = 9.2 if i % 2 == 0 else 7.0
        for off in (-0.16, 0.16):
            a = ang + off
            pt = QPointF(cx + rr * math.cos(a), cy + rr * math.sin(a))
            if path.elementCount() == 0:
                path.moveTo(pt)
            else:
                path.lineTo(pt)
    path.closeSubpath()
    p.drawPath(path)
    p.drawEllipse(QPointF(12, 12), 3.2, 3.2)


def _moon(p, c):
    path = QPainterPath()
    path.addEllipse(QPointF(12, 12), 8.5, 8.5)
    cut = QPainterPath()
    cut.addEllipse(QPointF(16.2, 8.6), 7.2, 7.2)
    p.drawPath(path.subtracted(cut))


def _sun(p, c):
    p.drawEllipse(QPointF(12, 12), 4.2, 4.2)
    for i in range(8):
        a = math.pi / 4 * i
        p.drawLine(QPointF(12 + 7.4 * math.cos(a), 12 + 7.4 * math.sin(a)),
                   QPointF(12 + 9.6 * math.cos(a), 12 + 9.6 * math.sin(a)))


def _bell(p, c):
    path = QPainterPath()
    path.moveTo(5.5, 17); path.quadTo(7.5, 15, 7.5, 10.5)
    path.cubicTo(7.5, 5.5, 16.5, 5.5, 16.5, 10.5)
    path.quadTo(16.5, 15, 18.5, 17); path.closeSubpath()
    p.drawPath(path)
    p.drawArc(QRectF(9.6, 17.6, 4.8, 3.6), 180 * 16, 180 * 16)
    p.drawLine(QPointF(12, 4.2), QPointF(12, 5.6))


def _bell_off(p, c):
    _bell(p, c)
    p.drawLine(QPointF(4, 4), QPointF(20, 20))


def _pin(p, c):
    path = QPainterPath()
    path.moveTo(12, 21)
    path.cubicTo(5.5, 14.5, 4.8, 12, 4.8, 9.6)
    path.cubicTo(4.8, 5.6, 8, 3, 12, 3)
    path.cubicTo(16, 3, 19.2, 5.6, 19.2, 9.6)
    path.cubicTo(19.2, 12, 18.5, 14.5, 12, 21)
    p.drawPath(path)
    p.drawEllipse(QPointF(12, 9.6), 2.6, 2.6)


def _globe(p, c):
    p.drawEllipse(QPointF(12, 12), 8.8, 8.8)
    p.drawEllipse(QPointF(12, 12), 3.8, 8.8)
    p.drawLine(QPointF(3.2, 12), QPointF(20.8, 12))
    p.drawArc(QRectF(4.8, 5.2, 14.4, 7), 0, 180 * 16)
    p.drawArc(QRectF(4.8, 11.8, 14.4, 7), 180 * 16, 180 * 16)


def _check(p, c):
    p.drawPath(_poly([(5, 12.5), (10, 17.5), (19, 7)]))


def _x(p, c):
    p.drawLine(QPointF(6, 6), QPointF(18, 18)); p.drawLine(QPointF(18, 6), QPointF(6, 18))


def _minus(p, c):
    p.drawLine(QPointF(6, 12), QPointF(18, 12))


def _plus(p, c):
    p.drawLine(QPointF(6, 12), QPointF(18, 12)); p.drawLine(QPointF(12, 6), QPointF(12, 18))


def _square(p, c):
    p.drawRoundedRect(QRectF(6, 6, 12, 12), 2, 2)


def _restore(p, c):
    p.drawRoundedRect(QRectF(5, 8.5, 10.5, 10.5), 2, 2)
    p.drawPath(_poly([(9, 8.5), (9, 5.5), (19, 5.5), (19, 15.5), (15.5, 15.5)]))


def _search(p, c):
    p.drawEllipse(QPointF(10.5, 10.5), 6.2, 6.2)
    p.drawLine(QPointF(15.2, 15.2), QPointF(20, 20))


def _chev_down(p, c):
    p.drawPath(_poly([(6.5, 9.5), (12, 15), (17.5, 9.5)]))


def _chev_right(p, c):
    p.drawPath(_poly([(9.5, 6.5), (15, 12), (9.5, 17.5)]))


def _chev_left(p, c):
    p.drawPath(_poly([(14.5, 6.5), (9, 12), (14.5, 17.5)]))


def _refresh(p, c):
    p.drawArc(QRectF(4.5, 4.5, 15, 15), 40 * 16, 270 * 16)
    p.drawPath(_poly([(17.2, 3.2), (18.4, 7.6), (14, 8.4)]))


def _play(p, c):
    p.setBrush(QBrush(c)); p.setPen(_pen(c, 1.2))
    p.drawPath(_poly([(8, 5.8), (18.5, 12), (8, 18.2)]))


def _stop(p, c):
    p.setBrush(QBrush(c)); p.setPen(_pen(c, 1.2))
    p.drawRoundedRect(QRectF(6.5, 6.5, 11, 11), 2, 2)


def _volume(p, c):
    p.drawPath(_poly([(4.5, 9.5), (8.2, 9.5), (12.5, 5.8), (12.5, 18.2), (8.2, 14.5), (4.5, 14.5)]))
    p.drawArc(QRectF(10, 8.2, 7.6, 7.6), -50 * 16, 100 * 16)
    p.drawArc(QRectF(8.2, 5.4, 12.4, 12.4 + 0.4), -52 * 16, 104 * 16)


def _clock(p, c):
    p.drawEllipse(QPointF(12, 12), 8.6, 8.6)
    p.drawPath(_poly([(12, 7), (12, 12), (15.4, 14)]))


def _compass(p, c):
    p.drawEllipse(QPointF(12, 12), 8.8, 8.8)
    p.setBrush(QBrush(c))
    p.drawPath(_poly([(15.8, 8.2), (13.4, 13.4), (8.2, 15.8), (10.6, 10.6)]))


def _snooze(p, c):
    p.drawEllipse(QPointF(12, 13), 7.4, 7.4)
    p.drawPath(_poly([(9.2, 11), (14.6, 11), (9.2, 15.2), (14.8, 15.2)]))
    p.drawLine(QPointF(4.2, 6), QPointF(7.2, 3.6)); p.drawLine(QPointF(19.8, 6), QPointF(16.8, 3.6))


def _mosque(p, c):
    p.drawArc(QRectF(6.5, 7, 11, 11), 0, 180 * 16)
    p.drawLine(QPointF(6.5, 12.5), QPointF(6.5, 20.5)); p.drawLine(QPointF(17.5, 12.5), QPointF(17.5, 20.5))
    p.drawLine(QPointF(4, 20.5), QPointF(20, 20.5))
    p.drawLine(QPointF(12, 7), QPointF(12, 4.2))
    p.drawLine(QPointF(10.6, 20.5), QPointF(10.6, 16.4)); p.drawLine(QPointF(13.4, 20.5), QPointF(13.4, 16.4))
    p.drawArc(QRectF(10.6, 14.6, 2.8, 3.6), 0, 180 * 16)


def _folder(p, c):
    p.drawPath(_poly([(3.8, 7.5), (3.8, 18.5), (20.2, 18.5), (20.2, 9), (11, 9), (9.4, 6.5), (3.8, 6.5), (3.8, 7.5)]))


def _locate(p, c):
    p.drawEllipse(QPointF(12, 12), 5.2, 5.2)
    p.setBrush(QBrush(c)); p.drawEllipse(QPointF(12, 12), 1.8, 1.8); p.setBrush(Qt.NoBrush)
    for a, b in (((12, 2.8), (12, 5.6)), ((12, 18.4), (12, 21.2)), ((2.8, 12), (5.6, 12)), ((18.4, 12), (21.2, 12))):
        p.drawLine(QPointF(*a), QPointF(*b))


def _database(p, c):
    p.drawEllipse(QRectF(5.5, 4.5, 13, 5)); p.drawArc(QRectF(5.5, 9.5, 13, 5), 180 * 16, 180 * 16)
    p.drawArc(QRectF(5.5, 14.5, 13, 5), 180 * 16, 180 * 16)
    p.drawLine(QPointF(5.5, 7), QPointF(5.5, 17)); p.drawLine(QPointF(18.5, 7), QPointF(18.5, 17))


def _palette(p, c):
    path = QPainterPath()
    path.moveTo(12, 3.2)
    path.cubicTo(6.4, 3.2, 3, 7, 3, 11.6)
    path.cubicTo(3, 16.4, 6.8, 20.4, 12, 20.4)
    path.cubicTo(13.6, 20.4, 14.2, 19.4, 13.6, 18.2)
    path.cubicTo(12.8, 16.6, 14, 15.4, 15.6, 15.4)
    path.lineTo(17.4, 15.4)
    path.cubicTo(19.6, 15.4, 21, 13.8, 21, 11.8)
    path.cubicTo(21, 6.8, 17, 3.2, 12, 3.2)
    p.drawPath(path)
    p.setBrush(QBrush(c)); p.setPen(Qt.NoPen)
    for x, y in ((7.8, 11), (10.2, 7.4), (14.6, 7.8)):
        p.drawEllipse(QPointF(x, y), 1.35, 1.35)


def _info(p, c):
    p.drawEllipse(QPointF(12, 12), 8.8, 8.8)
    p.drawLine(QPointF(12, 11), QPointF(12, 16.4))
    p.setBrush(QBrush(c)); p.drawEllipse(QPointF(12, 7.8), 1.1, 1.1); p.setBrush(Qt.NoBrush)


def _mail(p, c):
    p.drawRoundedRect(QRectF(3.4, 5.6, 17.2, 12.8), 2.6, 2.6)
    p.drawPath(_poly([(4.2, 7.2), (12, 13), (19.8, 7.2)]))


def _phone(p, c):
    p.drawRoundedRect(QRectF(7, 3, 10, 18), 2.6, 2.6)
    p.drawLine(QPointF(10.6, 17.8), QPointF(13.4, 17.8))


def _copy(p, c):
    p.drawRoundedRect(QRectF(8.6, 8.6, 11, 11), 2.4, 2.4)
    p.drawPath(_poly([(15.4, 8.6), (15.4, 5.8), (14.2, 4.6), (6.4, 4.6), (4.6, 6.4), (4.6, 14.2), (5.8, 15.4), (8.6, 15.4)]))


def _heart(p, c):
    path = QPainterPath(QPointF(12, 20))
    path.cubicTo(3.5, 14.2, 3.2, 9.6, 6.2, 7.2)
    path.cubicTo(8.4, 5.5, 11, 6.5, 12, 8.6)
    path.cubicTo(13, 6.5, 15.6, 5.5, 17.8, 7.2)
    path.cubicTo(20.8, 9.6, 20.5, 14.2, 12, 20)
    p.drawPath(path)


def _calendar(p, c):
    p.drawRoundedRect(QRectF(4, 5.6, 16, 14.6), 3, 3)
    p.drawLine(QPointF(4, 10.2), QPointF(20, 10.2))
    p.drawLine(QPointF(8.4, 3.6), QPointF(8.4, 7.4)); p.drawLine(QPointF(15.6, 3.6), QPointF(15.6, 7.4))
    p.setBrush(QBrush(c)); p.drawEllipse(QPointF(9, 14.4), 1, 1); p.drawEllipse(QPointF(15, 14.4), 1, 1); p.setBrush(Qt.NoBrush)


def _power(p, c):
    p.drawArc(QRectF(5, 5.4, 14, 14), 120 * 16, -300 * 16)
    p.drawLine(QPointF(12, 3.4), QPointF(12, 11.4))


_ICONS = {
    "calendar": _calendar, "power": _power,
    "info": _info, "mail": _mail, "phone": _phone, "copy": _copy, "heart": _heart,
    "palette": _palette,
    "home": _home, "gear": _gear, "moon": _moon, "sun": _sun, "bell": _bell, "bell_off": _bell_off, "pin": _pin,
    "globe": _globe, "check": _check, "x": _x, "minus": _minus, "plus": _plus, "square": _square, "restore": _restore,
    "search": _search, "chev_down": _chev_down, "chev_right": _chev_right, "chev_left": _chev_left, "refresh": _refresh,
    "play": _play, "stop": _stop, "volume": _volume, "clock": _clock, "compass": _compass, "snooze": _snooze,
    "mosque": _mosque, "folder": _folder, "locate": _locate, "db": _database,
}


def _onion_path(cx: float, base_y: float, r: float, h: float, pointy: float = 0.25) -> QPainterPath:
    path = QPainterPath(QPointF(cx - r, base_y))
    path.cubicTo(cx - r * 1.25, base_y - h * 0.5, cx - r * pointy, base_y - h * 0.78, cx, base_y - h)
    path.cubicTo(cx + r * pointy, base_y - h * 0.78, cx + r * 1.25, base_y - h * 0.5, cx + r, base_y)
    path.closeSubpath()
    return path


def _crescent_path(cx: float, cy: float, r: float, thick: float = 0.72, tilt: float = 0.0) -> QPainterPath:
    from PySide6.QtGui import QTransform
    outer = QPainterPath(); outer.addEllipse(QPointF(cx, cy), r, r)
    cut = QPainterPath(); off = r * (1 - thick) * 1.9
    cut.addEllipse(QPointF(cx + off, cy - off * 0.35), r * 0.9, r * 0.9)
    path = outer.subtracted(cut)
    tr = QTransform(); tr.translate(cx, cy); tr.rotate(tilt); tr.translate(-cx, -cy)
    return tr.map(path)


def logo_pixmap(size: int, dpr: float = 2.0) -> QPixmap:
    """Brand mark: a deep badge with a golden V made of two faceted strokes whose tops become minarets
    (balcony, onion dome, crescent finial), a glowing crescent cradled between them and a double
    eight-pointed star behind. Always drawn from the DARK variant of the active palette so it keeps
    its contrast on any background, while the metal colour follows the chosen theme."""
    from PySide6.QtGui import QLinearGradient, QRadialGradient
    from ..theme import T
    P = T.preview(T.palette, "dark")
    acc = P["accent"]; hi = QColor(acc).lighter(140); lo = QColor(P["accent2"]); deep = QColor(lo).darker(135)
    detail = size >= 44                      # tiny sizes (tray / title bar) drop the fine ornament
    pm = QPixmap(int(size * dpr), int(size * dpr)); pm.setDevicePixelRatio(dpr); pm.fill(Qt.transparent)
    p = QPainter(pm); p.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
    p.scale(size / 100.0, size / 100.0)

    def metal(y0: float, y1: float, a=hi, b=acc, c=deep) -> QBrush:
        g = QLinearGradient(0, y0, 0, y1); g.setColorAt(0, a); g.setColorAt(.5, b); g.setColorAt(1, c); return QBrush(g)

    # ---- badge
    badge = QPainterPath(); badge.addRoundedRect(QRectF(0, 0, 100, 100), 27, 27)
    g = QLinearGradient(0, 0, 100, 100); g.setColorAt(0, QColor(P["bg1"]).lighter(142)); g.setColorAt(1, QColor(P["bg0"]).darker(110))
    p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); p.drawPath(badge)
    p.save(); p.setClipPath(badge)
    glow = QRadialGradient(34, 14, 78); c0 = QColor(acc); c0.setAlpha(58); c1 = QColor(acc); c1.setAlpha(0)
    glow.setColorAt(0, c0); glow.setColorAt(1, c1); p.setBrush(QBrush(glow)); p.drawRect(QRectF(0, 0, 100, 100))
    # ---- double eight-pointed star (two squares, one rotated 45 deg)
    def star(half: float) -> QPainterPath:
        a = QPainterPath(); a.addRect(QRectF(50 - half, 51 - half, 2 * half, 2 * half))
        b = QPainterPath(); b.addRect(QRectF(50 - half, 51 - half, 2 * half, 2 * half))
        from PySide6.QtGui import QTransform
        tr = QTransform(); tr.translate(50, 51); tr.rotate(45); tr.translate(-50, -51)
        return a.united(tr.map(b))
    sc = QColor(acc); sc.setAlpha(205); p.setBrush(Qt.NoBrush); p.setPen(QPen(sc, 1.5)); p.drawPath(star(31))
    if detail:
        sc2 = QColor(acc); sc2.setAlpha(105); p.setPen(QPen(sc2, 1.1)); p.drawPath(star(25.5))
    p.restore()
    # ---- glowing crescent halo
    halo = QRadialGradient(50, 41, 17); h0 = QColor(hi); h0.setAlpha(110); h1 = QColor(hi); h1.setAlpha(0)
    halo.setColorAt(0, h0); halo.setColorAt(1, h1); p.setPen(Qt.NoPen); p.setBrush(QBrush(halo)); p.drawEllipse(QPointF(50, 41), 17, 17)
    # ---- minarets (behind the V)
    for cx in (30.75, 69.25):
        p.setPen(Qt.NoPen); p.setBrush(metal(12, 42))
        p.drawRect(QRectF(cx - 3.3, 27, 6.6, 15))                                   # shaft
        p.drawRoundedRect(QRectF(cx - 6.4, 29.6, 12.8, 3.4), 1.2, 1.2)             # balcony
        p.drawRect(QRectF(cx - 2.5, 21.5, 5.0, 9))                                   # upper shaft
        p.drawPath(_onion_path(cx, 22.4, 5.3, 10.6, 0.2))                           # dome
        p.setPen(QPen(acc, 1.3, Qt.SolidLine, Qt.RoundCap)); p.drawLine(QPointF(cx, 11.8), QPointF(cx, 9.6)); p.setPen(Qt.NoPen)
        if detail:
            p.setBrush(QBrush(hi)); p.drawPath(_crescent_path(cx, 7.7, 2.7, 0.7, -20))
            p.setBrush(QColor(P["bg0"])); 
            for k in (-1.6, 0, 1.6):                                                # balcony openings
                p.drawRect(QRectF(cx + k * 1.7 - 0.45, 30.6, 0.9, 1.4))
        else:
            p.setBrush(QBrush(hi)); p.drawEllipse(QPointF(cx, 9.4), 1.7, 1.7)
    # ---- the V: two faceted halves (light left, deeper right) for a 3-D metal look
    left = QPainterPath(QPointF(24, 40)); left.lineTo(50, 82); left.lineTo(50, 64); left.lineTo(37.5, 40); left.closeSubpath()
    right = QPainterPath(QPointF(76, 40)); right.lineTo(50, 82); right.lineTo(50, 64); right.lineTo(62.5, 40); right.closeSubpath()
    gl = QLinearGradient(24, 40, 50, 82); gl.setColorAt(0, hi); gl.setColorAt(.55, acc); gl.setColorAt(1, lo)
    gr = QLinearGradient(76, 40, 50, 82); gr.setColorAt(0, acc); gr.setColorAt(.6, lo); gr.setColorAt(1, deep)
    p.setPen(Qt.NoPen); p.setBrush(QBrush(gl)); p.drawPath(left); p.setBrush(QBrush(gr)); p.drawPath(right)
    if detail:                                                                       # light edge + centre seam
        e = QColor(255, 255, 255, 105); p.setPen(QPen(e, 0.9, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin))
        p.drawLine(QPointF(24.4, 40.4), QPointF(50, 81.4))
        s = QColor(0, 0, 0, 60); p.setPen(QPen(s, 0.8)); p.drawLine(QPointF(50, 64.5), QPointF(50, 81.5))
    # ---- crescent between the minarets
    p.setPen(Qt.NoPen); p.setBrush(metal(32, 50, hi, QColor(hi).darker(108), acc))
    p.drawPath(_crescent_path(50, 41.5, 8.2, 0.72, -28))
    if detail:
        p.setBrush(QBrush(hi)); p.drawEllipse(QPointF(53.6, 41.2), 1.1, 1.1)
    # ---- inner rim
    rim = QColor(acc); rim.setAlpha(90); p.setPen(QPen(rim, 1.0)); p.setBrush(Qt.NoBrush)
    p.drawRoundedRect(QRectF(0.6, 0.6, 98.8, 98.8), 26.5, 26.5)
    p.end()
    return pm
