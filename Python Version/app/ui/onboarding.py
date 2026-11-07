"""First-run wizard: language -> location -> method -> theme -> done."""
from __future__ import annotations
from PySide6.QtCore import QEasingCurve, QPointF, QRectF, Qt, QTimer, QVariantAnimation, Signal
from PySide6.QtGui import QBrush, QColor, QLinearGradient, QPainter, QPen
from PySide6.QtWidgets import QAbstractButton, QHBoxLayout, QSizePolicy, QVBoxLayout, QWidget

from .. import i18n, locations as L
from ..i18n import t
from ..methods import METHODS, ORDER, default_for
from ..state import AppState
from ..theme import T
from . import icons
from .widgets import Button, Label, SearchCombo, SlideStack, font

DOT_OF = {0: 0, 1: 1, 2: 1, 3: 1, 4: 2, 5: 3, 6: 4}


class OptionCard(QAbstractButton):
    def __init__(self, icon: str, title: str, sub: str = "", selected=False, glyph_text: str = "") -> None:
        super().__init__()
        self.icon, self.title, self.sub, self.sel, self.glyph_text = icon, title, sub, selected, glyph_text
        self.setFixedSize(250, 210); self.setCursor(Qt.PointingHandCursor); self.setAttribute(Qt.WA_Hover)
        self._h = 0.0; self._a = None

    def enterEvent(self, e): self._go(1); super().enterEvent(e)
    def leaveEvent(self, e): self._go(0); super().leaveEvent(e)

    def _go(self, v):
        if self._a: self._a.stop()
        a = QVariantAnimation(self); a.setStartValue(self._h); a.setEndValue(v); a.setDuration(200)
        a.valueChanged.connect(lambda x: (setattr(self, "_h", float(x)), self.update())); self._a = a; a.start()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(2, 2 + (1 - self._h) * 4, -2, -2)
        p.setPen(QPen(T.c("accent") if (self.sel or self._h > .6) else T.c("border_hi"), 2 if self.sel else 1))
        p.setBrush(T.c("surface_hi") if T.dark else QColor("#FFFFFF")); p.drawRoundedRect(r, 26, 26)
        g = QLinearGradient(0, 0, 64, 64); g.setColorAt(0, T.c("accent_hi")); g.setColorAt(1, T.c("accent2"))
        ir = QRectF(r.center().x() - 32, r.top() + 28, 64, 64)
        p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); p.drawRoundedRect(ir, 22, 22)
        if self.glyph_text:
            p.setPen(T.c("on_accent") if T.dark else QColor("#FFFFFF")); p.setFont(font(22, 800)); p.drawText(ir, Qt.AlignCenter, self.glyph_text)
        else:
            icons.draw(p, self.icon, ir.adjusted(16, 16, -16, -16), T.c("on_accent") if T.dark else QColor("#FFFFFF"), 2.1)
        p.setPen(T.c("text")); p.setFont(font(15, 800)); p.drawText(QRectF(r.left(), r.top() + 108, r.width(), 32), Qt.AlignCenter, self.title)
        p.setPen(T.c("text2")); p.setFont(font(10, 500)); p.drawText(QRectF(r.left() + 14, r.top() + 142, r.width() - 28, 44), Qt.AlignHCenter | Qt.AlignTop | Qt.TextWordWrap, self.sub)


class ThemeCard(QAbstractButton):
    def __init__(self, mode: str, title: str, selected: bool) -> None:
        super().__init__(); self.mode, self.title, self.sel = mode, title, selected
        self.setFixedSize(184, 190); self.setCursor(Qt.PointingHandCursor)

    def _mini(self, p, r, dark):
        bg, fg, card = (QColor("#0C1411"), QColor("#E6B85C"), QColor(255, 255, 255, 20)) if dark else (QColor("#FAF7F0"), QColor("#B8892B"), QColor("#FFFFFF"))
        p.setPen(Qt.NoPen); p.setBrush(bg); p.drawRoundedRect(r, 12, 12)
        p.setBrush(fg); p.drawRoundedRect(QRectF(r.left() + 10, r.top() + 10, r.width() * .45, 8), 4, 4)
        p.setBrush(card); p.drawRoundedRect(QRectF(r.left() + 10, r.top() + 26, r.width() - 20, 28), 8, 8)
        for i in range(3):
            p.setBrush(card); p.drawRoundedRect(QRectF(r.left() + 10 + i * (r.width() - 20) / 3, r.top() + 62, (r.width() - 20) / 3 - 5, r.height() - 72), 8, 8)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(2, 2, -2, -2)
        p.setPen(QPen(T.c("accent") if self.sel else T.c("border_hi"), 2 if self.sel else 1)); p.setBrush(T.c("surface_hi") if T.dark else QColor("#FFFFFF"))
        p.drawRoundedRect(r, 22, 22)
        pr = QRectF(r.left() + 14, r.top() + 14, r.width() - 28, 100)
        if self.mode == "auto":
            p.save(); p.setClipRect(QRectF(pr.left(), pr.top(), pr.width() / 2, pr.height())); self._mini(p, pr, True); p.restore()
            p.save(); p.setClipRect(QRectF(pr.center().x(), pr.top(), pr.width() / 2, pr.height())); self._mini(p, pr, False); p.restore()
        else:
            self._mini(p, pr, self.mode == "dark")
        p.setPen(T.c("text")); p.setFont(font(13, 800)); p.drawText(QRectF(r.left(), r.top() + 128, r.width(), 40), Qt.AlignCenter, self.title)


class Spinner(QWidget):
    def __init__(self) -> None:
        super().__init__(); self.setFixedSize(72, 72); self.a = 0
        self.t = QTimer(self); self.t.timeout.connect(lambda: (setattr(self, "a", (self.a + 8) % 360), self.update())); self.t.start(16)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        pen = QPen(T.c("border_hi"), 5); p.setPen(pen); p.drawEllipse(QRectF(8, 8, 56, 56))
        pen = QPen(T.c("accent"), 5); pen.setCapStyle(Qt.RoundCap); p.setPen(pen); p.drawArc(QRectF(8, 8, 56, 56), -self.a * 16, 100 * 16)


class SuccessMark(QWidget):
    def __init__(self) -> None:
        super().__init__(); self.setFixedSize(130, 130); self.v = 0.0
        self.an = QVariantAnimation(self); self.an.setStartValue(0.0); self.an.setEndValue(1.0); self.an.setDuration(700)
        self.an.setEasingCurve(QEasingCurve.OutBack); self.an.valueChanged.connect(lambda x: (setattr(self, "v", float(x)), self.update()))

    def play(self) -> None:
        self.an.start()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        s = max(0.0, self.v); c = QPointF(65, 65)
        p.translate(c); p.scale(s, s); p.translate(-c)
        g = QLinearGradient(0, 0, 130, 130); g.setColorAt(0, T.c("accent_hi")); g.setColorAt(1, T.c("accent2"))
        for i, a in enumerate((30, 18, 8)):
            p.setPen(Qt.NoPen); gg = T.c("accent", a / 100); p.setBrush(gg); p.drawEllipse(c, 52 + i * 6, 52 + i * 6)
        p.setBrush(QBrush(g)); p.drawEllipse(c, 50, 50)
        icons.draw(p, "check", QRectF(35, 35, 60, 60), T.c("on_accent") if T.dark else QColor("#FFFFFF"), 2.6)


class Onboarding(QWidget):
    finished = Signal()
    languageChanged = Signal(str)
    themeChanged = Signal(str)

    def __init__(self, state: AppState, locator) -> None:
        super().__init__()
        self.state, self.locator, self.s = state, locator, state.s
        self.mode = "manual"
        self._found = None
        self.dots = _Dots(); self.stack = SlideStack()
        lay = QVBoxLayout(self); lay.setContentsMargins(30, 10, 30, 30)
        lay.addWidget(self.dots, 0, Qt.AlignHCenter); lay.addWidget(self.stack, 1)
        self.build(0, animate=False)

    # ---- helpers ------------------------------------------------------
    def _page(self, title: str, sub: str, content: QWidget | None, back: int | None = None, nxt: tuple | None = None) -> QWidget:
        pg = QWidget(); outer = QVBoxLayout(pg); outer.setContentsMargins(0, 0, 0, 0)
        outer.addStretch(1)
        col = QVBoxLayout(); col.setSpacing(14); col.setAlignment(Qt.AlignHCenter)
        a = Label(title, 30, 800, "text", align=Qt.AlignCenter); b = Label(sub, 12, 500, "text2", align=Qt.AlignCenter); b.setWordWrap(True)
        col.addWidget(a); col.addWidget(b); col.addSpacing(14)
        if content is not None:
            col.addWidget(content, 0, Qt.AlignHCenter)
        if back is not None or nxt:
            col.addSpacing(18)
            row = QHBoxLayout(); row.setSpacing(12); row.addStretch(1)
            if back is not None:
                bb = Button(t("back"), "ghost", height=46); bb.setFixedWidth(120); bb.clicked.connect(lambda: self.go(back, -1)); row.addWidget(bb)
            if nxt:
                nb = Button(nxt[0], "primary", height=46); nb.setFixedWidth(200); nb.clicked.connect(nxt[1]); row.addWidget(nb)
            row.addStretch(1); col.addLayout(row)
        w = QWidget(); w.setFixedWidth(620); w.setLayout(col)
        h = QHBoxLayout(); h.addStretch(1); h.addWidget(w); h.addStretch(1)
        outer.addLayout(h); outer.addStretch(2)
        return pg

    def go(self, i: int, direction: int = 1, animate=True) -> None:
        self.dots.set(DOT_OF[i]); self.stack.go(i, direction, animate)

    def _cards(self, *cards) -> QWidget:
        w = QWidget(); l = QHBoxLayout(w); l.setSpacing(18); l.setContentsMargins(0, 0, 0, 0)
        for c in cards: l.addWidget(c)
        return w

    # ---- pages --------------------------------------------------------
    def build(self, start: int, animate=True) -> None:
        for pg in self.stack.pages: pg.deleteLater()
        self.stack.pages, self.stack.index = [], -1
        # 0 language
        ar = OptionCard("", "العربية", "واجهة كاملة من اليمين لليسار", i18n.lang() == "ar", "ع")
        en = OptionCard("", "English", "Full left-to-right interface", i18n.lang() == "en", "En")
        ar.clicked.connect(lambda: self._lang("ar")); en.clicked.connect(lambda: self._lang("en"))
        self.stack.add(self._page(t('ob_welcome'), "اختر لغتك · Choose your language", self._cards(ar, en)))
        # 1 mode
        a = OptionCard("locate", t("ob_auto"), t("ob_auto_sub")); m = OptionCard("globe", t("ob_manual"), t("ob_manual_sub"))
        a.clicked.connect(self._start_auto); m.clicked.connect(lambda: self._to_manual())
        self.stack.add(self._page(t("ob_how_loc"), "", self._cards(a, m), back=0))
        # 2 auto
        self.spin = Spinner(); self.found = Label("", 20, 800, "text", align=Qt.AlignCenter)
        self.found_sub = Label(t("detecting"), 12, 500, "text2", align=Qt.AlignCenter)
        box = QWidget(); bl = QVBoxLayout(box); bl.setAlignment(Qt.AlignHCenter); bl.addWidget(self.spin, 0, Qt.AlignHCenter); bl.addWidget(self.found); bl.addWidget(self.found_sub)
        row = QHBoxLayout(); self.yes = Button(t("ob_yes"), "primary", "check", height=46); self.edit = Button(t("ob_edit"), "ghost", height=46)
        self.yes.clicked.connect(self._accept_found); self.edit.clicked.connect(lambda: self._to_manual(True))
        row.addWidget(self.edit); row.addWidget(self.yes); self.btnrow = QWidget(); self.btnrow.setLayout(row); bl.addWidget(self.btnrow); self.btnrow.hide()
        self.stack.add(self._page(t("ob_found"), "", box, back=1))
        # 3 manual
        self.cb_country = SearchCombo(t("search"), t("no_results")); self.cb_region = SearchCombo(t("search"), t("no_results"))
        lang = i18n.lang()
        self.cb_country.set_items([(c.name(lang), c.code, c.code, f"{c.en} {c.ar} {c.code}".lower()) for c in L.countries()])
        self.cb_country.set_value(self.s.country_code); self._fill_regions()
        self.cb_country.changed.connect(lambda c: (setattr(self.s, "country_code", c), setattr(self.s, "region_index", 0), self._fill_regions()))
        self.cb_region.changed.connect(lambda i: setattr(self.s, "region_index", i))
        man = QWidget(); man.setFixedWidth(460); ml = QVBoxLayout(man); ml.setSpacing(14); ml.setContentsMargins(0, 0, 0, 0)
        ml.addWidget(Label(t("country"), 10.5, 600, "text2")); ml.addWidget(self.cb_country)
        ml.addWidget(Label(t("region"), 10.5, 600, "text2")); ml.addWidget(self.cb_region)
        self.stack.add(self._page(t("ob_pick_loc"), "", man, back=1, nxt=(t("next"), self._after_location)))
        # 4 method
        self.cb_method = SearchCombo(t("search"), t("no_results")); self._fill_methods()
        mw = QWidget(); mw.setFixedWidth(500); mv = QVBoxLayout(mw); mv.setContentsMargins(0, 0, 0, 0); mv.addWidget(self.cb_method)
        self.cb_method.changed.connect(lambda k: setattr(self.s, "method", "auto" if k == default_for(self.s.country_code) else k))
        self.stack.add(self._page(t("ob_method"), t("ob_method_sub"), mw, back=3, nxt=(t("next"), lambda: self.go(5))))
        # 5 theme
        cards = [ThemeCard(k, t(k), self.s.theme == k) for k in ("dark", "light", "auto")]
        for c in cards:
            c.clicked.connect(lambda _=False, k=c.mode: self._theme(k))
        self._theme_cards = cards
        self.stack.add(self._page(t("ob_theme"), "", self._cards(*cards), back=4, nxt=(t("next"), self._to_done)))
        # 6 done
        self.mark = SuccessMark()
        self.stack.add(self._page(t("ob_done"), t("ob_done_sub"), self.mark, nxt=(t("start"), self._finish)))
        self.stack.resize(self.stack.size())
        if start == 0:
            self.dots.set(0); self.stack.go(0, 1, False)
        else:
            self.stack.go(0, 1, False); QTimer.singleShot(60, lambda: self.go(start))

    def _fill_regions(self) -> None:
        lang = i18n.lang(); c = L.by_code(self.s.country_code)
        self.cb_region.set_items([(L.region_name(r, lang), "", i, f"{r.en} {r.ar}".lower()) for i, r in enumerate(c.regions)])
        self.cb_region.set_value(self.s.region_index)

    def _fill_methods(self) -> None:
        lang = i18n.lang(); d = default_for(self.s.country_code)
        self.cb_method.set_items([((METHODS[k].ar if lang == "ar" else METHODS[k].en) + (" ★" if k == d else ""), "", k, f"{METHODS[k].en} {METHODS[k].ar}".lower()) for k in ORDER])
        self.cb_method.set_value(d if self.s.method == "auto" else self.s.method)

    # ---- actions ------------------------------------------------------
    def _lang(self, code: str) -> None:
        self.languageChanged.emit(code)      # window applies language + direction, then rebuilds us

    def _theme(self, k: str) -> None:
        self.s.theme = k
        for c in self._theme_cards: c.sel = c.mode == k; c.update()
        self.themeChanged.emit(k)

    def _start_auto(self) -> None:
        self.mode = "auto"
        self.spin.show(); self.found.setText(""); self.found_sub.setText(t("detecting")); self.btnrow.hide()
        self.go(2)
        self.locator.detect(self._detected)

    def _detected(self, res) -> None:
        if not res:
            self.found.setText(""); self.found_sub.setText(t("ob_failed")); self.spin.hide()
            QTimer.singleShot(1600, lambda: self._to_manual(True)); return
        c, idx, fix = res
        self._found = res
        self.spin.hide(); r = c.regions[idx]
        self.found.setText(f"{L.region_name(r, i18n.lang())} · {c.name(i18n.lang())}")
        self.found_sub.setText(f"{fix.lat:.2f}, {fix.lng:.2f}  ({'GPS' if fix.source == 'gps' else 'IP'})")
        self.btnrow.show()

    def _accept_found(self) -> None:
        c, idx, fix = self._found
        self.s.country_code, self.s.region_index = c.code, idx
        self.s.lat, self.s.lng, self.s.tz = fix.lat, fix.lng, c.regions[idx].tz
        self.s.auto_location = True
        self._fill_methods(); self.go(4)

    def _to_manual(self, from_auto=False) -> None:
        self.mode = "manual"; self.s.auto_location = False
        if from_auto and self._found:
            self.s.country_code, self.s.region_index = self._found[0].code, self._found[1]
            self.cb_country.set_value(self.s.country_code); self._fill_regions()
        self.go(3)

    def _after_location(self) -> None:
        c = L.by_code(self.s.country_code); r = c.regions[min(self.s.region_index, len(c.regions) - 1)]
        self.s.lat, self.s.lng, self.s.tz = r.lat, r.lng, r.tz
        self._fill_methods(); self.go(4)

    def _to_done(self) -> None:
        self.go(6); QTimer.singleShot(350, self.mark.play)

    def _finish(self) -> None:
        self.s.first_run = False; self.s.save(); self.finished.emit()


class _Dots(QWidget):
    def __init__(self) -> None:
        super().__init__(); self.setFixedSize(150, 20); self.i = 0

    def set(self, i: int) -> None:
        self.i = i; self.update()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        x = 8
        for k in range(5):
            on = k == self.i; w = 28 if on else 8
            p.setPen(Qt.NoPen); p.setBrush(T.c("accent") if k <= self.i else T.c("border_hi"))
            p.drawRoundedRect(QRectF(x, 6, w, 8), 4, 4); x += w + 8
