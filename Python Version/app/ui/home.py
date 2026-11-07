"""Home page: header, hero (next prayer + progress ring) and the six prayer cards."""
from __future__ import annotations
import math
from datetime import datetime

from PySide6.QtCore import QEasingCurve, QPointF, QRectF, QSize, Qt, QTimer, QVariantAnimation, Signal
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QFontMetrics, QLinearGradient, QPainter, QPainterPath,
                           QPen, QPixmap, QRadialGradient)
from PySide6.QtWidgets import (QAbstractButton, QHBoxLayout, QScrollArea, QSizePolicy, QVBoxLayout, QWidget, QFrame)

from .. import i18n
from ..config import PRAYER_KEYS
from ..i18n import t
from ..state import AppState
from ..theme import T
from . import art, icons
from .widgets import Button, IconButton, Label, Segmented, font, StatusBadge


# ---------------------------------------------------------------------------
def arch_path(r: QRectF, radius: float = 26) -> QPainterPath:
    """Pointed (mihrab) arch on top, soft rounded corners on the bottom."""
    w = r.width()
    shoulder = r.top() + w * 0.56
    cx = r.center().x()
    p = QPainterPath(QPointF(r.left(), r.bottom() - radius))
    p.lineTo(r.left(), shoulder)
    p.cubicTo(r.left(), shoulder - w * 0.30, cx - w * 0.20, r.top() + w * 0.12, cx, r.top())
    p.cubicTo(cx + w * 0.20, r.top() + w * 0.12, r.right(), shoulder - w * 0.30, r.right(), shoulder)
    p.lineTo(r.right(), r.bottom() - radius)
    p.quadTo(r.right(), r.bottom(), r.right() - radius, r.bottom())
    p.lineTo(r.left() + radius, r.bottom())
    p.quadTo(r.left(), r.bottom(), r.left(), r.bottom() - radius)
    p.closeSubpath()
    return p


class PrayerCard(QAbstractButton):
    alarmToggled = Signal(str)

    def __init__(self, key: str, time_text: str, state: str, lang: str, alarm_on: bool = True, parent=None) -> None:
        super().__init__(parent)
        self.alarm_on = alarm_on
        self.key, self.time_text, self.state, self.lang = key, time_text, state, lang   # state: next|past|future
        self.setMinimumSize(118, 250)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.setAttribute(Qt.WA_Hover); self.setCursor(Qt.PointingHandCursor)
        self._h = 0.0; self._ha: QVariantAnimation | None = None
        self._pm: QPixmap | None = None
        self._pm_size = QSize()
        self._rerender = QTimer(self); self._rerender.setSingleShot(True); self._rerender.setInterval(140)
        self._rerender.timeout.connect(self._render_now)
        self._glow = 0.0

    def set_glow(self, v: float) -> None:
        self._glow = v
        if self.state == "next": self.update()

    # hover lift
    def enterEvent(self, e): self._hv(1); super().enterEvent(e)
    def leaveEvent(self, e): self._hv(0); super().leaveEvent(e)

    def _hv(self, v: float) -> None:
        if self._ha: self._ha.stop()
        a = QVariantAnimation(self); a.setStartValue(self._h); a.setEndValue(v); a.setDuration(240)
        a.setEasingCurve(QEasingCurve.OutCubic); a.valueChanged.connect(lambda x: (setattr(self, "_h", float(x)), self.update()))
        self._ha = a; a.start()

    def resizeEvent(self, e) -> None:
        self._rerender.start()
        super().resizeEvent(e)

    def _bell_rect(self) -> QRectF:
        r = self._art_rect()
        return QRectF(r.center().x() - 16, r.top() + r.width() * 0.50 + 34, 32, 32)

    def mouseMoveEvent(self, e) -> None:
        self.setCursor(Qt.PointingHandCursor)
        super().mouseMoveEvent(e)

    def mouseReleaseEvent(self, e) -> None:
        if e.button() == Qt.LeftButton and self._bell_rect().contains(e.position()) and self.rect().contains(e.position().toPoint()):
            self.alarm_on = not self.alarm_on; self.update(); self.alarmToggled.emit(self.key)
            self.setDown(False)
            return
        super().mouseReleaseEvent(e)

    def _art_rect(self) -> QRectF:
        return QRectF(self.rect()).adjusted(8, 18, -8, -14)

    def _render_now(self) -> None:
        r = self._art_rect()
        dpr = self.devicePixelRatioF()
        self._pm = art.render(self.key, max(60, int(r.width())), max(120, int(r.height())), dpr)
        self._pm_size = self.size()
        self.update()

    def paintEvent(self, _e) -> None:
        if self._pm is None:
            self._render_now()
        p = QPainter(self)
        p.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        base = self._art_rect()
        lift = 9 * self._h
        r = base.translated(0, -lift)
        # grow slightly on hover
        grow = 5 * self._h
        r = r.adjusted(-grow / 2, -grow / 2, grow / 2, grow / 2)
        nxt = self.state == "next"
        # shadow
        sh = QColor(0, 0, 0)
        for i in range(10):
            sh.setAlphaF(min(1.0, (0.22 if T.dark else 0.075) * (1 - i / 10) ** 1.6 * (1 + self._h * 0.7)))
            p.setPen(Qt.NoPen); p.setBrush(sh)
            p.drawPath(arch_path(r.adjusted(-i * 1.3, i * 1.2 + 4, i * 1.3, i * 2.2 + 6), 26 + i))
        if nxt:   # golden glow
            g = QColor(T.c("accent")); g.setAlphaF((0.16 + 0.10 * self._glow) * (1 if T.dark else 0.8))
            for i in range(8):
                g.setAlphaF(((0.16 + 0.10 * self._glow) * (1 - i / 8)) * (1 if T.dark else 0.7))
                p.setBrush(g); p.drawPath(arch_path(r.adjusted(-i * 1.8, -i * 1.8, i * 1.8, i * 1.8), 26 + i * 2))
        path = arch_path(r, 26)
        p.save(); p.setClipPath(path)
        if self._pm:
            p.drawPixmap(r, self._pm, QRectF(0, 0, self._pm.width(), self._pm.height()))
        # legibility gradient over the lower part
        g = QLinearGradient(0, r.top() + r.height() * 0.52, 0, r.bottom())
        g.setColorAt(0, QColor(0, 0, 0, 0)); g.setColorAt(0.55, QColor(0, 0, 0, 120)); g.setColorAt(1, QColor(0, 0, 0, 215))
        p.fillRect(r, QBrush(g))
        # top sheen
        sg = QLinearGradient(0, r.top(), 0, r.top() + r.height() * 0.35)
        sg.setColorAt(0, QColor(255, 255, 255, 34)); sg.setColorAt(1, QColor(255, 255, 255, 0))
        p.fillRect(r, QBrush(sg))
        if self.state == "past":
            p.fillRect(r, QColor(8, 12, 10, 105))
        p.restore()
        # border
        if nxt:
            bg = QLinearGradient(r.topLeft(), r.bottomRight()); bg.setColorAt(0, T.c("accent_hi")); bg.setColorAt(1, T.c("accent2"))
            p.setPen(QPen(QBrush(bg), 2.2))
        else:
            p.setPen(QPen(QColor(255, 255, 255, 60 if T.dark else 150), 1.2))
        p.setBrush(Qt.NoBrush); p.drawPath(path)
        # texts
        ar, sec = t(self.key), i18n.tl(self.key, "en" if i18n.lang() == "ar" else "ar")
        pad = 10
        tx = QRectF(r.left() + pad, r.bottom() - 112, r.width() - 2 * pad, 112)
        op = 0.62 if self.state == "past" else 1.0
        p.setOpacity(op)
        p.setPen(QColor("#FFFFFF")); p.setFont(font(max(14, min(21, r.width() / 8.2)), 800))
        p.drawText(QRectF(tx.left(), tx.top() + 8, tx.width(), 36), Qt.AlignCenter, ar)
        p.setPen(T.c("accent_hi")); p.setFont(font(10.5, 500))
        p.drawText(QRectF(tx.left(), tx.top() + 42, tx.width(), 20), Qt.AlignCenter, sec)
        p.setPen(QColor("#FFFFFF")); p.setFont(font(max(15, min(22, r.width() / 7.4)), 800))
        p.drawText(QRectF(tx.left(), tx.top() + 64, tx.width(), 38), Qt.AlignCenter, self.time_text)
        p.setOpacity(1)
        # alarm bell (click to mute / unmute this prayer's alarm)
        br0 = self._bell_rect().translated(0, -lift)
        p.setPen(QPen(QColor(255, 255, 255, 70), 1)); p.setBrush(QColor(0, 0, 0, 105 if self.alarm_on else 140)); p.drawEllipse(br0)
        icons.draw(p, "bell" if self.alarm_on else "bell_off", br0.adjusted(7, 7, -7, -7),
                   T.c("accent_hi") if self.alarm_on else QColor(255, 255, 255, 150), 2.0)
        # badge
        if nxt:
            txt = t("upcoming")
            fm = QFontMetrics(font(9, 800)); bw = fm.horizontalAdvance(txt) + 26
            br = QRectF(r.center().x() - bw / 2, r.top() + r.width() * 0.50, bw, 24)
            gg = QLinearGradient(br.topLeft(), br.bottomRight()); gg.setColorAt(0, T.c("accent_hi")); gg.setColorAt(1, T.c("accent2"))
            p.setPen(Qt.NoPen); p.setBrush(QBrush(gg)); p.drawRoundedRect(br, 12, 12)
            p.setPen(T.c("on_accent") if T.dark else QColor("#FFFFFF")); p.setFont(font(9, 800)); p.drawText(br, Qt.AlignCenter, txt)
        elif self.state == "past":
            cr = QRectF(r.center().x() - 15, r.top() + r.width() * 0.50, 30, 30)
            p.setPen(Qt.NoPen); p.setBrush(QColor(0, 0, 0, 120)); p.drawEllipse(cr)
            icons.draw(p, "check", cr.adjusted(7, 7, -7, -7), QColor("#5BC49A"), 2.6)


# ---------------------------------------------------------------------------
class GlassChip(QWidget):
    """Small frosted pill with an optional icon, painted over imagery."""
    def __init__(self, icon: str = "", size: float = 10.5, weight: int = 700, accent: bool = False) -> None:
        super().__init__()
        self.icon, self.fs, self.fw, self.accent, self.text = icon, size, weight, accent, ""
        self.setFixedHeight(int(size * 1.333) + 20)

    def set_text(self, text: str) -> None:
        self.text = text
        self.setFixedWidth(QFontMetrics(font(self.fs, self.fw)).horizontalAdvance(text) + 34 + (24 if self.icon else 0))
        self.update()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        if self.accent:
            g = QLinearGradient(r.topLeft(), r.bottomRight()); g.setColorAt(0, T.c("accent_hi")); g.setColorAt(1, T.c("accent2"))
            p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); fg = QColor("#1A1405") if T.dark or True else QColor("#FFFFFF")
            fg = T.c("on_accent") if T.dark else QColor("#FFFFFF")
        else:
            p.setPen(QPen(QColor(255, 255, 255, 80), 1)); p.setBrush(QColor(255, 255, 255, 36)); fg = QColor("#FFFFFF")
        p.drawRoundedRect(r, r.height() / 2, r.height() / 2)
        p.setPen(fg); p.setFont(font(self.fs, self.fw))
        rtl = self.isRightToLeft()
        if self.icon:
            ix = r.right() - 28 if rtl else r.left() + 12
            icons.draw(p, self.icon, QRectF(ix, r.center().y() - 8, 16, 16), fg, 2.0)
            tr = QRectF(r.left() + 12, 0, r.width() - 42, r.height()) if rtl else QRectF(r.left() + 32, 0, r.width() - 42, r.height())
        else:
            tr = r
        p.drawText(tr, Qt.AlignCenter, self.text)


class CountdownBlock(QWidget):
    """HH : MM : SS as three frosted tiles with a progress bar underneath."""
    W, H = 372, 178

    def __init__(self) -> None:
        super().__init__()
        self.setFixedSize(self.W, self.H)
        self.hms = (0, 0, 0)
        self.frac = 0.0
        self.caption = ""
        self.prev_name = self.next_name = ""

    def set_values(self, hms, frac: float, caption: str, prev_name: str, next_name: str) -> None:
        self.hms, self.frac, self.caption, self.prev_name, self.next_name = hms, max(0.0, min(1.0, frac)), caption, prev_name, next_name
        self.update()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        w = self.W
        p.setPen(QColor(255, 255, 255, 215)); p.setFont(font(10.5, 700))
        p.drawText(QRectF(0, 0, w, 22), Qt.AlignCenter, self.caption)
        tw, th, gap = 102, 88, 15
        x0 = (w - (3 * tw + 2 * gap)) / 2
        labels = (t("unit_h"), t("unit_m"), t("unit_s"))
        f = font(34, 800); fm = QFontMetrics(f)
        cell = fm.horizontalAdvance("0") + 1
        # one colour family: soft tint (hours) -> bright accent (minutes) -> deep accent (seconds)
        acc_hi, acc, acc_lo = T.c("accent_hi"), T.c("accent"), T.c("accent2")
        deep_hi, deep_lo = QColor(acc_lo), QColor(acc_lo).darker(155)
        for i, val in enumerate(self.hms):
            r = QRectF(x0 + i * (tw + gap), 30, tw, th)
            if i == 0:          # hours: frosted glass with a faint theme tint
                tint = QColor(acc); tint.setAlpha(46)
                sheet = QLinearGradient(r.topLeft(), r.bottomLeft())
                sheet.setColorAt(0, QColor(255, 255, 255, 58)); sheet.setColorAt(1, QColor(255, 255, 255, 20))
                p.setPen(QPen(T.c("accent_hi", 0.55), 1)); p.setBrush(QBrush(sheet)); p.drawRoundedRect(r, 20, 20)
                p.setPen(Qt.NoPen); p.setBrush(tint); p.drawRoundedRect(r, 20, 20)
                digit, label_col = QColor("#FFFFFF"), QColor(255, 255, 255, 195)
            else:
                hi_c, lo_c = (acc_hi, acc_lo) if i == 1 else (deep_hi, deep_lo)
                g = QLinearGradient(r.topLeft(), r.bottomRight()); g.setColorAt(0, hi_c); g.setColorAt(1, lo_c)
                p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); p.drawRoundedRect(r, 20, 20)
                mid = QColor((hi_c.red() + lo_c.red()) // 2, (hi_c.green() + lo_c.green()) // 2, (hi_c.blue() + lo_c.blue()) // 2)
                digit = T.on_color(mid)
                label_col = QColor(digit); label_col.setAlpha(190)
            p.setFont(f); p.setPen(digit)
            txt = f"{val:02d}"; tot = cell * len(txt); x = r.center().x() - tot / 2
            for ch in txt:
                p.drawText(QRectF(x, r.top() + 10, cell, 46), Qt.AlignCenter, ch); x += cell
            p.setFont(font(8.5, 700)); p.setPen(label_col)
            p.drawText(QRectF(r.left(), r.bottom() - 26, r.width(), 20), Qt.AlignCenter, labels[i])
            if i < 2:
                p.setPen(Qt.NoPen); p.setBrush(QColor(255, 255, 255, 150))
                cx = r.right() + gap / 2
                p.drawEllipse(QPointF(cx, r.center().y() - 7), 2, 2); p.drawEllipse(QPointF(cx, r.center().y() + 7), 2, 2)
        # progress
        by = 30 + th + 20
        bar = QRectF(x0, by, 3 * tw + 2 * gap, 8)
        p.setPen(Qt.NoPen); p.setBrush(QColor(255, 255, 255, 52)); p.drawRoundedRect(bar, 4, 4)
        rtl = self.isRightToLeft()
        fw = max(8.0, bar.width() * self.frac)
        fr = QRectF(bar.right() - fw if rtl else bar.left(), bar.top(), fw, bar.height())
        g = QLinearGradient(fr.left(), 0, fr.right(), 0); g.setColorAt(0, T.c("accent2")); g.setColorAt(1, T.c("accent_hi"))
        p.setBrush(QBrush(g)); p.drawRoundedRect(fr, 4, 4)
        kx = fr.left() if rtl else fr.right()
        p.setBrush(QColor(255, 255, 255, 60)); p.drawEllipse(QPointF(kx, bar.center().y()), 10, 10)
        p.setBrush(QColor("#FFFFFF")); p.drawEllipse(QPointF(kx, bar.center().y()), 5, 5)
        p.setFont(font(9, 600)); p.setPen(QColor(255, 255, 255, 170))
        fm9 = QFontMetrics(p.font())
        left_r = QRectF(bar.left(), by + 14, bar.width() / 2, 18); right_r = QRectF(bar.center().x(), by + 14, bar.width() / 2, 18)
        if rtl:      # progress runs right -> left: previous prayer on the right, next on the left
            p.drawText(right_r, Qt.AlignRight | Qt.AlignVCenter | Qt.AlignAbsolute, self.prev_name)
            p.drawText(left_r, Qt.AlignLeft | Qt.AlignVCenter | Qt.AlignAbsolute, self.next_name)
        else:
            p.drawText(left_r, Qt.AlignLeft | Qt.AlignVCenter | Qt.AlignAbsolute, self.prev_name)
            p.drawText(right_r, Qt.AlignRight | Qt.AlignVCenter | Qt.AlignAbsolute, self.next_name)


class HeroCard(QFrame):
    """Next-prayer banner: the prayer's own scene as a wide backdrop, name + time on the leading side,
    frosted countdown tiles on the trailing side."""
    def __init__(self, state: AppState, parent=None) -> None:
        super().__init__(parent)
        self.state = state
        self.key = "fajr"
        self._pm: QPixmap | None = None
        self._pm_key = ""
        self._rerender = QTimer(self); self._rerender.setSingleShot(True); self._rerender.setInterval(150)
        self._rerender.timeout.connect(self._render_now)
        self.setMinimumHeight(236); self.setMaximumHeight(262)
        lay = QHBoxLayout(self); lay.setContentsMargins(38, 24, 34, 24); lay.setSpacing(24)
        col = QVBoxLayout(); col.setSpacing(10)
        self.pill = GlassChip("bell", 10, 700, accent=True)
        self.name = Label("", 48, 800, "#FFFFFF", align=Qt.AlignLeading | Qt.AlignVCenter)
        self.chip = GlassChip("clock", 13, 700)
        row = QHBoxLayout(); row.setContentsMargins(0, 0, 0, 0); row.addWidget(self.chip); row.addStretch(1)
        prow = QHBoxLayout(); prow.setContentsMargins(0, 0, 0, 0); prow.addWidget(self.pill); prow.addStretch(1)
        col.addStretch(1); col.addLayout(prow); col.addWidget(self.name); col.addLayout(row); col.addStretch(1)
        lay.addLayout(col, 1)
        self.countdown = CountdownBlock(); lay.addWidget(self.countdown, 0, Qt.AlignVCenter)

    def set_prayer(self, key: str, time_text: str) -> None:
        if key != self.key or self._pm is None:
            self.key = key; self._render_now()
        self.pill.set_text(t("next_prayer"))
        self.name.setText(t(key))
        mark = "\u200f" if i18n.is_rtl() else "\u200e"
        self.chip.set_text(f"{mark}{time_text}   ·   {i18n.tl(key, 'en' if i18n.lang() == 'ar' else 'ar')}")
        self.update()

    def resizeEvent(self, e) -> None:
        self._rerender.start(); super().resizeEvent(e)

    def _render_now(self) -> None:
        w, h = max(300, self.width()), max(160, self.height())
        self._pm = art.render(self.key, w, h, self.devicePixelRatioF(), unit=h * 1.0, cx=w * 0.5)
        self._pm_key = self.key; self.update()

    def paintEvent(self, _e) -> None:
        if self._pm is None:
            self._render_now()
        p = QPainter(self); p.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        r = QRectF(self.rect()).adjusted(1, 1, -1, -1)
        path = QPainterPath(); path.addRoundedRect(r, 30, 30)
        if T.dark:
            for i in range(6):
                p.setPen(Qt.NoPen); p.setBrush(QColor(0, 0, 0, int(26 * (1 - i / 6))))
                p.drawRoundedRect(r.adjusted(-i, -i + 3, i, i + 5), 30 + i, 30 + i)
        p.save(); p.setClipPath(path)
        if self._pm:
            p.drawPixmap(r, self._pm, QRectF(0, 0, self._pm.width(), self._pm.height()))
        rtl = self.isRightToLeft()
        lead, trail = (r.right(), r.left()) if rtl else (r.left(), r.right())
        g = QLinearGradient(lead, 0, trail, 0)
        g.setColorAt(0, QColor(6, 8, 14, 190)); g.setColorAt(0.38, QColor(6, 8, 14, 95)); g.setColorAt(0.62, QColor(6, 8, 14, 0))
        p.fillRect(r, QBrush(g))
        g2 = QLinearGradient(trail, 0, lead, 0)
        g2.setColorAt(0, QColor(6, 8, 14, 165)); g2.setColorAt(0.36, QColor(6, 8, 14, 70)); g2.setColorAt(0.6, QColor(6, 8, 14, 0))
        p.fillRect(r, QBrush(g2))
        sh = QLinearGradient(0, r.top(), 0, r.top() + 60); sh.setColorAt(0, QColor(255, 255, 255, 34)); sh.setColorAt(1, QColor(255, 255, 255, 0))
        p.fillRect(r, QBrush(sh))
        p.restore()
        bp = QLinearGradient(r.topLeft(), r.bottomRight()); bp.setColorAt(0, T.c("accent_hi", 0.75)); bp.setColorAt(1, T.c("accent2", 0.25))
        p.setPen(QPen(QBrush(bp), 1.4)); p.setBrush(Qt.NoBrush); p.drawRoundedRect(r, 30, 30)


# ---------------------------------------------------------------------------
class HomePage(QWidget):
    openSettings = Signal()
    languageChanged = Signal(str)
    themeToggle = Signal()
    snoozeCancel = Signal()

    def __init__(self, state: AppState, notifier, parent=None) -> None:
        super().__init__(parent)
        self.state, self.notifier = state, notifier
        self._cards: list[PrayerCard] = []
        self._last_next = ""
        root = QVBoxLayout(self); root.setContentsMargins(30, 14, 30, 22); root.setSpacing(18)

        # header
        head = QHBoxLayout(); head.setSpacing(12)
        col = QVBoxLayout(); col.setSpacing(2)
        top = QHBoxLayout(); top.setSpacing(10)
        self.pin = QLabelIcon("pin", 22); top.addWidget(self.pin)
        self.city = Label("", 27, 800, "text"); top.addWidget(self.city); top.addStretch()
        col.addLayout(top)
        self.dateline = Label("", 11.5, 500, "text2", align=Qt.AlignLeading | Qt.AlignVCenter); col.addWidget(self.dateline)
        head.addLayout(col, 1)
        self.snooze_chip = Button("", "ghost", "snooze", height=40); self.snooze_chip.hide()
        self.snooze_chip.clicked.connect(self.snoozeCancel.emit)
        head.addWidget(self.snooze_chip)
        self.lang = Segmented([("ar", "عربي"), ("en", "EN")], i18n.lang(), height=40, min_seg=58)
        self.lang.changed.connect(self.languageChanged.emit)
        head.addWidget(self.lang)
        self.theme_btn = IconButton("sun", 40, radius=14)
        self.theme_btn.clicked.connect(self.themeToggle.emit)
        head.addWidget(self.theme_btn)
        root.addLayout(head)

        self.hero = HeroCard(state); root.addWidget(self.hero)

        self.scroll = QScrollArea(); self.scroll.setWidgetResizable(True); self.scroll.setFrameShape(QFrame.NoFrame)
        self.scroll.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.row_host = QWidget(); self.row = QHBoxLayout(self.row_host); self.row.setContentsMargins(0, 0, 0, 0); self.row.setSpacing(6)
        self.scroll.setWidget(self.row_host)
        root.addWidget(self.scroll, 1)

        self._glow_a = QVariantAnimation(self); self._glow_a.setStartValue(0.0); self._glow_a.setEndValue(1.0)
        self._glow_a.setDuration(2600); self._glow_a.setLoopCount(-1); self._glow_a.setEasingCurve(QEasingCurve.InOutSine)
        self._glow_a.valueChanged.connect(self._glow); self._glow_a.start()

        self.state.alertsChanged.connect(self._sync_bells)
        self.refresh()

    def _sync_bells(self) -> None:
        for c in self._cards:
            c.alarm_on = bool(self.state.s.alerts.get(c.key, True)); c.update()

    def _toggle_alarm(self, key: str) -> None:
        s = self.state.s
        s.alerts[key] = not s.alerts.get(key, True); s.save()
        self.state.toast.emit(("alarm_on" if s.alerts[key] else "alarm_off") + "|" + key, "ok" if s.alerts[key] else "warn")
        self.state.alertsChanged.emit()

    def _glow(self, v) -> None:
        for c in self._cards:
            c.set_glow(float(v))

    def restyle(self) -> None:
        for w in (self.city, self.dateline):
            w.restyle()
        self.pin.update()
        self.theme_btn.set_icon("sun" if T.dark else "moon")

    # ---- data -> widgets --------------------------------------------------
    def refresh(self) -> None:
        st, snap = self.state, self.state.snapshot()
        c, r = st.country, st.region
        self.city.setText(r.ar if i18n.lang() == "ar" else r.en)
        now = st.now()
        self.dateline.setText(f"{c.name(i18n.lang())}   ·   {i18n.fmt_date(now.date(), st.hijri(now.date()))}")
        self.theme_btn.set_icon("sun" if T.dark else "moon")
        self.lang.set_current(i18n.lang(), animate=False)
        tm = {k: dt.strftime("%H:%M") for k, dt in snap.today}
        self.hero.set_prayer(snap.next_key, snap.next_dt.strftime("%H:%M"))
        while self.row.count():
            it = self.row.takeAt(0); w = it.widget()
            if w:
                w.hide(); w.setParent(None); w.deleteLater()
        self._cards = []
        for k in PRAYER_KEYS:
            state = "next" if k == snap.next_key else ("past" if k in snap.past else "future")
            card = PrayerCard(k, tm[k], state, i18n.lang(), bool(self.state.s.alerts.get(k, True)))
            card.alarmToggled.connect(self._toggle_alarm)
            self._cards.append(card); self.row.addWidget(card, 1)
        self._last_next = snap.next_key
        self.tick()

    def tick(self) -> None:
        snap = self.state.snapshot()
        now = self.state.now()
        secs = max(0, int((snap.next_dt - now).total_seconds()))
        h, rem = divmod(secs, 3600); m, s = divmod(rem, 60)
        total = max(1.0, (snap.next_dt - snap.prev_dt).total_seconds())
        frac = 1 - secs / total
        self.hero.countdown.set_values((h, m, s), frac, t("until", name=t(snap.next_key)), t(snap.prev_key), t(snap.next_key))
        if snap.next_key != self._last_next:
            self.refresh()
        sn = self.notifier.next_snooze()
        if sn:
            left = max(0, int((sn[1] - now).total_seconds()))
            self.snooze_chip.setText(f"{t('snoozed')} · {left // 60:02d}:{left % 60:02d}")
            self.snooze_chip.setFixedWidth(max(130, self.snooze_chip.sizeHint().width())); self.snooze_chip.show()
        else:
            self.snooze_chip.hide()


class QLabelIcon(QWidget):
    def __init__(self, name: str, size: int, color="accent", parent=None) -> None:
        super().__init__(parent)
        self.name, self.color = name, color
        self.setFixedSize(size, size)

    def paintEvent(self, _e) -> None:
        p = QPainter(self)
        icons.draw(p, self.name, QRectF(self.rect()), T.c(self.color), 2.0)
