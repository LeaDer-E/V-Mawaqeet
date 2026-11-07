"""Themes page: Dark / Light / Auto plus a gallery of color palettes."""
from __future__ import annotations
from PySide6.QtCore import QEasingCurve, QRectF, Qt, QVariantAnimation, Signal
from PySide6.QtGui import QBrush, QColor, QLinearGradient, QPainter, QPen
from PySide6.QtWidgets import QAbstractButton, QColorDialog, QFrame, QGridLayout, QScrollArea, QVBoxLayout, QWidget

from .. import i18n
from ..i18n import t
from ..theme import PALETTES, T
from . import icons
from .home import arch_path
from .widgets import Label, Segmented, font


class PaletteCard(QAbstractButton):
    W, H = 232, 196

    def __init__(self, name: str, selected: bool) -> None:
        super().__init__()
        self.name, self.sel, self._h = name, selected, 0.0
        self.setFixedSize(self.W, self.H); self.setCursor(Qt.PointingHandCursor); self.setAttribute(Qt.WA_Hover)
        self._a = None

    def enterEvent(self, e): self._go(1); super().enterEvent(e)
    def leaveEvent(self, e): self._go(0); super().leaveEvent(e)

    def _go(self, v: float) -> None:
        if self._a: self._a.stop()
        a = QVariantAnimation(self); a.setStartValue(self._h); a.setEndValue(v); a.setDuration(180); a.setEasingCurve(QEasingCurve.OutCubic)
        a.valueChanged.connect(lambda x: (setattr(self, "_h", float(x)), self.update())); self._a = a; a.start()

    def paintEvent(self, _e) -> None:
        P = T.preview(self.name, T.mode)          # preview in the CURRENT mode (dark or light)
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(3, 3 + (1 - self._h) * 3, -3, -3)
        # outer card (follows the active palette so the gallery itself looks native)
        p.setPen(QPen(T.c("accent") if self.sel else T.c("border_hi"), 2.2 if self.sel else 1))
        p.setBrush(T.c("surface_hi") if T.dark else QColor("#FFFFFF")); p.drawRoundedRect(r, 24, 24)
        # mini window
        m = QRectF(r.left() + 12, r.top() + 12, r.width() - 24, 112)
        g = QLinearGradient(m.topLeft(), m.bottomRight()); g.setColorAt(0, P["bg0"]); g.setColorAt(1, P["bg1"])
        p.setPen(QPen(P["border_hi"], 1)); p.setBrush(QBrush(g)); p.drawRoundedRect(m, 16, 16)
        # sidebar strip + header
        p.setPen(Qt.NoPen); p.setBrush(P["bg2"]); p.drawRoundedRect(QRectF(m.left() + 6, m.top() + 6, 14, m.height() - 12), 7, 7)
        ag = QLinearGradient(0, 0, 1, 1); ag = QLinearGradient(m.left(), 0, m.right(), 0); ag.setColorAt(0, P["accent"]); ag.setColorAt(1, P["accent2"])
        p.setBrush(QBrush(ag)); p.drawRoundedRect(QRectF(m.left() + 9, m.top() + 10, 8, 8), 3, 3)
        p.drawRoundedRect(QRectF(m.left() + 30, m.top() + 10, m.width() * 0.34, 8), 4, 4)
        # hero strip
        hs = QRectF(m.left() + 28, m.top() + 26, m.width() - 38, 26)
        hg = QLinearGradient(hs.left(), 0, hs.right(), 0); hg.setColorAt(0, P["accent2"]); hg.setColorAt(1, P["accent"])
        hc = QColor(P["accent"]); hc.setAlphaF(0.30)
        p.setBrush(hc); p.drawRoundedRect(hs, 9, 9)
        p.setBrush(QBrush(hg)); p.drawRoundedRect(QRectF(hs.left() + 8, hs.top() + 8, hs.width() * 0.35, 10), 5, 5)
        # three arches
        aw = (m.width() - 38 - 12) / 3
        for i in range(3):
            ar = QRectF(m.left() + 28 + i * (aw + 6), m.top() + 60, aw, 44)
            fill = QColor(P["text"]); fill.setAlphaF(0.10)
            p.setPen(QPen(P["accent"], 1.6) if i == 1 else Qt.NoPen); p.setBrush(fill); p.drawPath(arch_path(ar, 8))
        # label + swatches
        p.setPen(T.c("text")); p.setFont(font(11.5, 800))
        ar_, en_, *_ = PALETTES.get(self.name) or ("لونك الخاص", "Your own color")
        label = ar_ if i18n.lang() == "ar" else en_
        lr = QRectF(r.left() + 72, r.top() + 132, r.width() - 88, 36) if i18n.is_rtl() else QRectF(r.left() + 16, r.top() + 132, r.width() - 88, 36)
        p.drawText(lr, (Qt.AlignRight if i18n.is_rtl() else Qt.AlignLeft) | Qt.AlignVCenter | Qt.AlignAbsolute, label)
        sx = r.left() + 16 if i18n.is_rtl() else r.right() - 16 - 44
        for i, c in enumerate((P["accent"], P["accent2"])):
            p.setPen(QPen(QColor(255, 255, 255, 80) if T.dark else QColor(0, 0, 0, 40), 1)); p.setBrush(c)
            p.drawEllipse(QRectF(sx + i * 22, r.top() + 141, 18, 18))
        if self.name == "custom":      # rainbow "pick any colour" badge
            from PySide6.QtGui import QConicalGradient
            cg = QConicalGradient(m.center(), 0)
            for i, col in enumerate(("#FF5C5C", "#FFC24A", "#7BE06A", "#4AC8FF", "#8A7BFF", "#FF5CC8", "#FF5C5C")):
                cg.setColorAt(i / 6, QColor(col))
            p.setPen(QPen(QColor(255, 255, 255, 220), 2.4)); p.setBrush(QBrush(cg)); p.drawEllipse(m.center(), 17, 17)
            p.setPen(Qt.NoPen); p.setBrush(P["bg0"]); p.drawEllipse(m.center(), 6, 6)
        if self.sel:
            cr = QRectF(r.right() - 36, r.top() + 18, 24, 24) if True else QRectF()
            g2 = QLinearGradient(cr.topLeft(), cr.bottomRight()); g2.setColorAt(0, T.c("accent_hi")); g2.setColorAt(1, T.c("accent2"))
            p.setPen(Qt.NoPen); p.setBrush(QBrush(g2)); p.drawEllipse(cr)
            icons.draw(p, "check", cr.adjusted(5, 5, -5, -5), T.c("on_accent") if T.dark else QColor("#FFFFFF"), 2.6)


class ThemesPage(QWidget):
    modeChanged = Signal(str)
    paletteChanged = Signal(str)
    customChosen = Signal(str)

    def __init__(self, settings, parent=None) -> None:
        super().__init__(parent)
        self.s = settings
        outer = QVBoxLayout(self); outer.setContentsMargins(30, 14, 30, 0); outer.setSpacing(14)
        outer.addWidget(Label(t("themes"), 27, 800, "text"))
        outer.addWidget(Label(t("theme_hint"), 11.5, 500, "text2"))
        outer.addSpacing(4)
        outer.addWidget(Label(t("theme_mode"), 12, 700, "accent"))
        self.mode = Segmented([("dark", t("dark")), ("light", t("light")), ("auto", t("auto"))], settings.theme,
                              icons_={"dark": "moon", "light": "sun", "auto": "compass"}, min_seg=150)
        self.mode.changed.connect(self.modeChanged.emit)
        outer.addWidget(self.mode, 0, Qt.AlignLeading)
        outer.addSpacing(6)
        outer.addWidget(Label(t("palette"), 12, 700, "accent"))
        self.scroll = QScrollArea(); self.scroll.setWidgetResizable(True); self.scroll.setFrameShape(QFrame.NoFrame)
        self.host = QWidget(); self.grid = QGridLayout(self.host); self.grid.setContentsMargins(0, 6, 8, 24)
        self.grid.setHorizontalSpacing(16); self.grid.setVerticalSpacing(16)
        self.scroll.setWidget(self.host); outer.addWidget(self.scroll, 1)
        self.cards = [PaletteCard(n, n == settings.palette) for n in ("custom", *PALETTES)]
        for c in self.cards:
            c.clicked.connect(lambda _=False, n=c.name: self._pick(n))
        self._cols = 0
        self._flow()

    def _pick(self, name: str) -> None:
        if name == "custom":
            col = QColorDialog.getColor(QColor(self.s.custom_accent), self.window(), t("custom_color"))
            if col.isValid():
                self.customChosen.emit(col.name().upper())
            return
        if name == self.s.palette:
            return
        for c in self.cards:
            c.sel = c.name == name; c.update()
        self.paletteChanged.emit(name)

    def resizeEvent(self, e) -> None:
        self._flow(); super().resizeEvent(e)

    def _flow(self) -> None:
        avail = max(300, self.width() - 60 - self.contentsMargins().left() - self.contentsMargins().right())
        cols = max(1, int(avail // (PaletteCard.W + 16)))
        if cols == self._cols:
            return
        self._cols = cols
        while self.grid.count():
            self.grid.takeAt(0)
        for i, c in enumerate(self.cards):
            self.grid.addWidget(c, i // cols, i % cols, Qt.AlignLeft | Qt.AlignTop)
        for c in range(cols + 1):
            self.grid.setColumnStretch(c, 1 if c == cols else 0)
        rows = (len(self.cards) + cols - 1) // cols
        for rr in range(rows + 1):
            self.grid.setRowStretch(rr, 1 if rr == rows else 0)
