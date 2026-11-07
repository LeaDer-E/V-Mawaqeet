"""About dialog: an in-window overlay (dimmed backdrop + rounded card) that follows theme, palette and language."""
from __future__ import annotations
from PySide6.QtCore import QEasingCurve, QPoint, QPropertyAnimation, QRectF, QUrl, Qt, Signal
from PySide6.QtGui import (QBrush, QColor, QDesktopServices, QGuiApplication, QLinearGradient, QPainter, QPainterPath, QPen)
from PySide6.QtWidgets import (QFrame, QGraphicsOpacityEffect, QHBoxLayout, QScrollArea, QVBoxLayout, QWidget)

from .. import APP_NAME, APP_VERSION, DEV_EMAIL, DEV_NAME, DEV_PHONE, i18n
from ..i18n import t
from ..theme import T
from . import art, icons
from .widgets import IconButton, Label, font

HEADER = 124


class _Divider(QWidget):
    """Thin line with a small diamond ornament in the middle."""
    def __init__(self) -> None:
        super().__init__(); self.setFixedHeight(18)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        cy, w = self.height() / 2, self.width()
        for x0, x1 in ((0, w / 2 - 16), (w / 2 + 16, w)):
            g = QLinearGradient(x0, 0, x1, 0)
            a, b = T.c("accent", 0.0), T.c("accent", 0.55)
            g.setColorAt(0, a if x0 == 0 else b); g.setColorAt(1, b if x0 == 0 else a)
            p.setPen(QPen(QBrush(g), 1.2)); p.drawLine(x0, cy, x1, cy)
        p.setPen(Qt.NoPen); p.setBrush(T.c("accent"))
        path = QPainterPath(); c = w / 2
        path.moveTo(c, cy - 5); path.lineTo(c + 5, cy); path.lineTo(c, cy + 5); path.lineTo(c - 5, cy); path.closeSubpath()
        p.drawPath(path)


class _ContactRow(QFrame):
    def __init__(self, icon: str, text: str, copy_cb, open_cb=None, tip_open: str = "") -> None:
        super().__init__()
        self.icon, self.text = icon, text
        self.setFixedHeight(50)
        lay = QHBoxLayout(self); lay.setContentsMargins(66, 0, 10, 0); lay.setSpacing(6)
        lay.addStretch(1)
        if open_cb:
            b = IconButton("mail", 38, tip=tip_open, radius=12); b.clicked.connect(open_cb); lay.addWidget(b)
        c = IconButton("copy", 38, tip=t("copy"), radius=12); c.clicked.connect(copy_cb); lay.addWidget(c)
        self.setLayoutDirection(Qt.LeftToRight)        # contact details are always LTR

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        p.setPen(QPen(T.c("border"), 1)); p.setBrush(T.c("surface_hi") if T.dark else QColor(27, 36, 32, 10))
        p.drawRoundedRect(r, 16, 16)
        ic = QRectF(16, r.center().y() - 17, 34, 34)
        g = QLinearGradient(ic.topLeft(), ic.bottomRight()); g.setColorAt(0, T.c("accent_hi")); g.setColorAt(1, T.c("accent2"))
        p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); p.drawRoundedRect(ic, 11, 11)
        icons.draw(p, self.icon, ic.adjusted(8, 8, -8, -8), T.c("on_accent") if T.dark else QColor("#FFFFFF"), 2.0)
        p.setPen(T.c("text")); p.setFont(font(11.5, 600))
        p.setLayoutDirection(Qt.LeftToRight)          # keep "+20 128 ..." in natural order inside an RTL app
        p.drawText(QRectF(60, 0, self.width() - 160, self.height()), Qt.AlignLeft | Qt.AlignVCenter | Qt.AlignAbsolute, self.text)


class _Card(QWidget):
    def __init__(self, scene_key: str, w: int, h: int, dpr: float) -> None:
        super().__init__()
        self.setFixedSize(w, h)
        self._scene = art.render(scene_key, w, HEADER, dpr, unit=HEADER * 1.05, cx=w * 0.5)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform)
        pad = 14
        r = QRectF(self.rect()).adjusted(pad, pad, -pad, -pad)
        for i in range(pad):
            p.setPen(Qt.NoPen); p.setBrush(QColor(0, 0, 0, int((46 if T.dark else 30) * (1 - i / pad) ** 1.6)))
            p.drawRoundedRect(r.adjusted(-i, -i + 6, i, i + 6), 30 + i, 30 + i)
        path = QPainterPath(); path.addRoundedRect(r, 30, 30)
        p.setPen(QPen(T.c("border_hi"), 1))
        p.setBrush(T.c("bg1").lighter(118) if T.dark else QColor("#FFFFFF")); p.drawPath(path)
        p.save(); p.setClipPath(path)
        hr = QRectF(r.left(), r.top(), r.width(), HEADER)
        p.drawPixmap(hr, self._scene, QRectF(0, 0, self._scene.width(), self._scene.height()))
        fade = QLinearGradient(0, hr.bottom() - 52, 0, hr.bottom())
        base = T.c("bg1").lighter(118) if T.dark else QColor("#FFFFFF")
        c0 = QColor(base); c0.setAlpha(0); fade.setColorAt(0, c0); fade.setColorAt(1, base)
        p.fillRect(QRectF(hr.left(), hr.bottom() - 52, hr.width(), 53), QBrush(fade))
        p.restore()
        # logo badge overlapping the header edge
        bx, by = r.center().x() - 36, hr.bottom() - 40
        p.setPen(QPen(T.c("bg1").lighter(118) if T.dark else QColor("#FFFFFF"), 5)); p.setBrush(Qt.NoBrush)
        p.drawRoundedRect(QRectF(bx, by, 72, 72), 24, 24)
        p.drawPixmap(QRectF(bx, by, 72, 72), icons.logo_pixmap(72), QRectF(0, 0, 144, 144))


class AboutDialog(QWidget):
    closed = Signal()

    def __init__(self, parent: QWidget, scene_key: str, toast_cb) -> None:
        super().__init__(parent)
        self.toast_cb = toast_cb
        self.setFocusPolicy(Qt.StrongFocus)
        self.card: _Card | None = None
        self.scene_key = scene_key
        self.setAttribute(Qt.WA_NoSystemBackground, False)
        self._fx = QGraphicsOpacityEffect(self); self.setGraphicsEffect(self._fx)

    # ---- layout ----------------------------------------------------------------
    def _build(self) -> None:
        pw, ph = self.width(), self.height()
        cw = min(600, max(380, pw - 40)); ch = min(800, max(440, ph - 30))
        self.card = _Card(self.scene_key, cw, ch, self.devicePixelRatioF()); self.card.setParent(self)
        lay = QVBoxLayout(self.card); lay.setContentsMargins(14, 14, 14, 14); lay.setSpacing(0)
        lay.addSpacing(HEADER - 14 + 4)
        sc = QScrollArea(); sc.setWidgetResizable(True); sc.setFrameShape(QFrame.NoFrame)
        sc.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        body = QWidget(); col = QVBoxLayout(body); col.setContentsMargins(34, 40, 34, 16); col.setSpacing(9)
        mark = "\u200f" if i18n.is_rtl() else "\u200e"      # force paragraph direction (text may start with a Latin word)
        col.addWidget(Label(APP_NAME, 25, 800, "text", align=Qt.AlignCenter))
        col.addWidget(Label(f"{t('about_kicker')}   ·   {t('version')} {APP_VERSION}", 9.5, 700, "accent", align=Qt.AlignCenter))
        col.addSpacing(2)
        for key, size, color, weight in (("about_body", 11.5, "text", 500),):
            lb = Label(mark + t(key), size, weight, color, align=Qt.AlignCenter); lb.setWordWrap(True); col.addWidget(lb)
        col.addWidget(_Divider())
        d1 = Label(mark + t("about_dua"), 11.5, 600, "accent", align=Qt.AlignCenter); d1.setWordWrap(True); col.addWidget(d1)
        d2 = Label(t("about_amen"), 12, 800, "accent", align=Qt.AlignCenter); col.addWidget(d2)
        col.addWidget(_Divider())
        col.addWidget(Label(t("developer"), 9.5, 700, "text2", align=Qt.AlignCenter))
        col.addWidget(Label(DEV_NAME, 18, 800, "text", align=Qt.AlignCenter))
        col.addSpacing(2)
        col.addWidget(_ContactRow("mail", DEV_EMAIL, lambda: self._copy(DEV_EMAIL), lambda: QDesktopServices.openUrl(QUrl(f"mailto:{DEV_EMAIL}")), t("send_mail")))
        col.addWidget(_ContactRow("phone", DEV_PHONE, lambda: self._copy(DEV_PHONE)))
        col.addSpacing(2)
        d3 = Label(mark + t("about_dua2"), 10.5, 600, "text2", align=Qt.AlignCenter); d3.setWordWrap(True); col.addWidget(d3)
        col.addSpacing(4)
        col.addWidget(Label(mark + t("made_with"), 9.5, 500, "text3", align=Qt.AlignCenter))
        heart = Label(f"{mark}♥  {t('made_care')}", 9.5, 600, "text2", align=Qt.AlignCenter); heart.setWordWrap(True); col.addWidget(heart)
        sc.setWidget(body); lay.addWidget(sc, 1)
        x = IconButton("x", 36, radius=12, glyph=16); x.fixed_color = QColor("#FFFFFF"); x.setParent(self.card)
        x.move(cw - 14 - 14 - 36, 14 + 12); x.clicked.connect(self.close_animated); x.raise_()
        self.card.move((pw - cw) // 2, (ph - ch) // 2)

    def open(self) -> None:
        self.setGeometry(self.parentWidget().rect()); self._build(); self.show(); self.raise_(); self.setFocus()
        a = QPropertyAnimation(self._fx, b"opacity", self); a.setStartValue(0.0); a.setEndValue(1.0); a.setDuration(220); a.start(); self._a = a
        end = self.card.pos(); self.card.move(end + QPoint(0, 26))
        b = QPropertyAnimation(self.card, b"pos", self); b.setStartValue(end + QPoint(0, 26)); b.setEndValue(end); b.setDuration(320); b.setEasingCurve(QEasingCurve.OutCubic); b.start(); self._b = b

    def close_animated(self) -> None:
        a = QPropertyAnimation(self._fx, b"opacity", self); a.setStartValue(1.0); a.setEndValue(0.0); a.setDuration(170)
        a.finished.connect(self._done); a.start(); self._a = a

    def _done(self) -> None:
        self.hide(); self.closed.emit(); self.deleteLater()

    def _copy(self, text: str) -> None:
        QGuiApplication.clipboard().setText(text); self.toast_cb(t("copied"))

    # ---- events ------------------------------------------------------------------
    def keyPressEvent(self, e) -> None:
        if e.key() == Qt.Key_Escape:
            self.close_animated()
        else:
            super().keyPressEvent(e)

    def mousePressEvent(self, e) -> None:
        if self.card and not self.card.geometry().adjusted(14, 14, -14, -14).contains(e.position().toPoint()):
            self.close_animated()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        m = self.parentWidget().layout().contentsMargins().left() if self.parentWidget() and self.parentWidget().layout() else 0
        r = QRectF(self.rect()).adjusted(m, m, -m, -m)
        p.setPen(Qt.NoPen); p.setBrush(QColor(4, 8, 6, 175) if T.dark else QColor(30, 22, 8, 120))
        p.drawRoundedRect(r, 20 if m else 0, 20 if m else 0)
