"""Frameless always-on-top notification with the prayer's own scene as its backdrop."""
from __future__ import annotations
from PySide6.QtCore import (QEasingCurve, QParallelAnimationGroup, QPoint, QPropertyAnimation, QRectF, Qt, QTimer, Signal)
from PySide6.QtGui import (QBrush, QColor, QGuiApplication, QIntValidator, QLinearGradient, QPainter, QPainterPath, QPen, QPixmap)
from PySide6.QtWidgets import QHBoxLayout, QLineEdit, QVBoxLayout, QWidget

from .. import i18n
from ..i18n import t
from ..theme import T
from . import art, icons
from .widgets import Button, IconButton, Label, font

W, H, H_SHORT, PAD = 420, 372, 262, 10          # PAD = transparent margin for the shadow


class NotificationPopup(QWidget):
    stopped = Signal()
    snoozed = Signal(str, int)
    openApp = Signal()

    def __init__(self) -> None:
        super().__init__(None, Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setAttribute(Qt.WA_ShowWithoutActivating)
        self.setFixedSize(W, H)
        self.key = "fajr"
        self._bg: QPixmap | None = None
        self._auto = QTimer(self); self._auto.setSingleShot(True); self._auto.timeout.connect(self.dismiss)
        self._grp: QParallelAnimationGroup | None = None
        self.root = QVBoxLayout(self); self.root.setContentsMargins(PAD + 18, PAD + 16, PAD + 18, PAD + 18); self.root.setSpacing(10)
        self.setStyleSheet("""
            QLineEdit { background: rgba(255,255,255,0.16); color:#FFFFFF; border:1px solid rgba(255,255,255,0.34);
                        border-radius: 13px; padding: 4px 10px; selection-background-color: rgba(255,255,255,0.4); }
            QLineEdit:focus { border:1px solid rgba(255,255,255,0.85); }""")

    # ---- building ---------------------------------------------------------
    def _clear(self, lay) -> None:
        while lay.count():
            it = lay.takeAt(0)
            if it.widget():
                it.widget().hide(); it.widget().deleteLater()
            elif it.layout():
                self._clear(it.layout())

    def _build(self, title: str, name: str, sub: str, snooze_default: int, with_snooze: bool = True) -> None:
        self._clear(self.root)
        top = QHBoxLayout(); top.setSpacing(12)
        top.addWidget(_BellBadge())
        col = QVBoxLayout(); col.setSpacing(0)
        a = Label(title, 10.5, 700, "#FFFFFF"); b = Label(name, 26, 800, "#FFFFFF"); c = Label(sub, 10.5, 500, "rgba(255,255,255,0.82)")
        a.setStyleSheet("color:#F4DFA6; background:transparent;")
        for w in (a, b, c): col.addWidget(w)
        top.addLayout(col, 1)
        x = IconButton("x", 32, radius=10, glyph=15); x.fixed_color = QColor("#FFFFFF"); x.clicked.connect(self.stop)
        top.addWidget(x, 0, Qt.AlignTop)
        self.root.addLayout(top)
        self.root.addStretch(1)
        row = QHBoxLayout(); row.setSpacing(10)
        stop = Button(t("stop"), "primary", "stop", height=44); stop.clicked.connect(self.stop)
        opn = Button(t("open_app"), "glass", height=44); opn.clicked.connect(self._open)
        row.addWidget(stop, 1); row.addWidget(opn, 1); self.root.addLayout(row)
        if not with_snooze:                       # iqama is a moment, not something to postpone
            return
        sn = QHBoxLayout(); sn.setSpacing(8)
        for m in (5, 10, 15):
            btn = Button(t("snooze_n", n=m), "glass", "snooze", height=38)
            btn.clicked.connect(lambda _=False, mm=m: self._snooze(mm)); sn.addWidget(btn, 1)
        self.root.addLayout(sn)
        cu = QHBoxLayout(); cu.setSpacing(8)
        self.minutes = QLineEdit(str(snooze_default)); self.minutes.setValidator(QIntValidator(1, 720, self))
        self.minutes.setAlignment(Qt.AlignCenter); self.minutes.setFixedSize(76, 38); self.minutes.setFont(font(11.5, 700))
        self.minutes.setPlaceholderText(t("snooze_ph")); self.minutes.returnPressed.connect(self._snooze_custom)
        go = Button(f"{t('snooze_custom')}", "glass", "snooze", height=38); go.clicked.connect(self._snooze_custom)
        unit = Label(t("minutes"), 10, 600, "rgba(255,255,255,0.85)")
        cu.addWidget(self.minutes); cu.addWidget(unit); cu.addWidget(go, 1)
        self.root.addLayout(cu)

    def present(self, key: str, time_text: str, kind: str, pre_minutes: int = 0, snooze_default: int = 5) -> None:
        self.key = key
        if kind == "pre":
            title = t("pre_title", name=t(key), n=pre_minutes) if pre_minutes else t("next_prayer")
            name = t(key)
        elif kind == "snooze":
            title, name = t("snoozed"), t(key)
        elif kind == "iqama":
            title, name = t("iqama_title"), t(key)
        else:
            title, name = t("time_for"), t(key)
        mark = "\u200f" if i18n.is_rtl() else "\u200e"
        with_snooze = kind != "iqama"
        self.setFixedSize(W, H if with_snooze else H_SHORT)
        self._build(title, name, f"{mark}{time_text}  ·  {i18n.tl(key, 'en' if i18n.lang() == 'ar' else 'ar')}", snooze_default, with_snooze)
        dpr = self.devicePixelRatioF()
        iw, ih = W - 2 * PAD, self.height() - 2 * PAD
        self._bg = art.render(key, iw, ih, dpr, unit=ih * 0.78, cx=iw * 0.5)
        scr = QGuiApplication.primaryScreen().availableGeometry()
        end = QPoint(scr.right() - self.width() - 14, scr.bottom() - self.height() - 14)
        start = end + QPoint(60, 0)
        self.move(start); self.setWindowOpacity(0.0); self.show(); self.raise_()
        g = QParallelAnimationGroup(self)
        a = QPropertyAnimation(self, b"pos"); a.setStartValue(start); a.setEndValue(end); a.setDuration(420); a.setEasingCurve(QEasingCurve.OutCubic)
        o = QPropertyAnimation(self, b"windowOpacity"); o.setStartValue(0.0); o.setEndValue(1.0); o.setDuration(320)
        g.addAnimation(a); g.addAnimation(o); g.start(); self._grp = g
        self._auto.start(60_000 if kind in ("pre", "iqama") else 180_000)

    # ---- actions ----------------------------------------------------------
    def dismiss(self) -> None:
        self._auto.stop()
        g = QParallelAnimationGroup(self)
        a = QPropertyAnimation(self, b"pos"); a.setStartValue(self.pos()); a.setEndValue(self.pos() + QPoint(60, 0)); a.setDuration(260)
        o = QPropertyAnimation(self, b"windowOpacity"); o.setStartValue(self.windowOpacity()); o.setEndValue(0.0); o.setDuration(260)
        g.addAnimation(a); g.addAnimation(o); g.finished.connect(self.hide); g.start(); self._grp = g

    def stop(self) -> None:
        self.stopped.emit(); self.dismiss()

    def _snooze(self, m: int) -> None:
        self.snoozed.emit(self.key, max(1, int(m))); self.dismiss()

    def _snooze_custom(self) -> None:
        txt = self.minutes.text().strip()
        if txt.isdigit() and int(txt) > 0:
            self._snooze(int(txt))

    def _open(self) -> None:
        self.openApp.emit(); self.dismiss()

    # ---- painting ---------------------------------------------------------
    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        r = QRectF(self.rect()).adjusted(PAD, PAD, -PAD, -PAD)
        for i in range(PAD):
            p.setPen(Qt.NoPen); p.setBrush(QColor(0, 0, 0, int(34 * (1 - i / PAD) ** 1.5)))
            p.drawRoundedRect(r.adjusted(-i, -i + 4, i, i + 4), 28 + i, 28 + i)
        path = QPainterPath(); path.addRoundedRect(r, 28, 28)
        p.save(); p.setClipPath(path)
        if self._bg:
            p.drawPixmap(r, self._bg, QRectF(0, 0, self._bg.width(), self._bg.height()))
        g = QLinearGradient(0, r.top(), 0, r.bottom())
        g.setColorAt(0, QColor(4, 6, 12, 120)); g.setColorAt(0.30, QColor(4, 6, 12, 40)); g.setColorAt(0.52, QColor(4, 6, 12, 150)); g.setColorAt(1, QColor(4, 6, 12, 225))
        p.fillRect(r, QBrush(g))
        p.restore()
        hl = QLinearGradient(r.left(), 0, r.right(), 0)
        c0 = T.c("accent_hi", 0.0); c1 = T.c("accent_hi", 0.9)
        hl.setColorAt(0, c0); hl.setColorAt(.5, c1); hl.setColorAt(1, c0)
        p.setPen(QPen(QColor(255, 255, 255, 60), 1)); p.setBrush(Qt.NoBrush); p.drawRoundedRect(r, 28, 28)
        p.setPen(QPen(QBrush(hl), 2)); p.drawLine(r.left() + 50, r.top() + 1, r.right() - 50, r.top() + 1)


class _BellBadge(QWidget):
    def __init__(self) -> None:
        super().__init__(); self.setFixedSize(54, 54)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        g = QLinearGradient(0, 0, 54, 54); g.setColorAt(0, T.c("accent_hi")); g.setColorAt(1, T.c("accent2"))
        p.setPen(QPen(QColor(255, 255, 255, 90), 1)); p.setBrush(QBrush(g)); p.drawRoundedRect(QRectF(.5, .5, 53, 53), 18, 18)
        icons.draw(p, "bell", QRectF(14, 14, 26, 26), T.c("on_accent") if T.dark else QColor("#FFFFFF"), 2.0)
