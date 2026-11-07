"""Custom-painted widgets: nothing here looks like a stock OS control."""
from __future__ import annotations
from typing import Callable, Sequence

from PySide6.QtCore import (QEasingCurve, QPoint, QPointF, QPropertyAnimation, QRectF, QSize, Qt, QTimer,
                            QVariantAnimation, Signal, Property, QParallelAnimationGroup)
from PySide6.QtGui import (QColor, QFont, QFontMetrics, QLinearGradient, QPainter, QPainterPath, QPen, QBrush)
from PySide6.QtWidgets import (QAbstractButton, QFrame, QGraphicsOpacityEffect, QLabel, QLineEdit, QListWidget,
                               QListWidgetItem, QVBoxLayout, QWidget, QGraphicsDropShadowEffect, QSizePolicy, QApplication)

from ..theme import T
from . import icons

FAMILY = "Tajawal"


def font(size: float = 10.5, weight: int = 500) -> QFont:
    f = QFont(FAMILY)
    f.setPixelSize(int(round(size * 1.333)))
    f.setWeight(QFont.Weight(weight))
    return f


def _anim(target: float, cb: Callable[[float], None], start: float, ms=180) -> QVariantAnimation:
    a = QVariantAnimation()
    a.setStartValue(start); a.setEndValue(target); a.setDuration(ms)
    a.setEasingCurve(QEasingCurve.OutCubic)
    a.valueChanged.connect(cb)
    return a


class Hoverable:
    """Mixin: smooth 0..1 hover value in self._h"""
    def _init_hover(self) -> None:
        self._h = 0.0
        self._ha: QVariantAnimation | None = None

    def _hover_to(self, v: float) -> None:
        if self._ha:
            self._ha.stop()
        self._ha = _anim(v, self._set_h, self._h, 160)
        self._ha.start()

    def _set_h(self, v) -> None:
        self._h = float(v)
        self.update()


# ---------------------------------------------------------------------------
class Card(QFrame):
    """Glass surface card with a rounded border (and soft shadow in light mode)."""
    def __init__(self, radius: int = 22, parent=None, pad: int = 0) -> None:
        super().__init__(parent)
        self.radius = radius
        self.setAttribute(Qt.WA_StyledBackground, False)
        if pad:
            self.setContentsMargins(pad, pad, pad, pad)

    def paintEvent(self, _e) -> None:
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        if not T.dark:
            for i, a in enumerate((10, 7, 4)):
                p.setPen(Qt.NoPen); p.setBrush(QColor(60, 45, 15, a))
                p.drawRoundedRect(r.adjusted(-i * 1.2, i * 0.4, i * 1.2, i * 2.4 + 1), self.radius + i, self.radius + i)
        p.setPen(QPen(T.c("border"), 1)); p.setBrush(T.c("surface"))
        p.drawRoundedRect(r, self.radius, self.radius)


class Label(QLabel):
    def __init__(self, text="", size=11, weight=500, color="text", parent=None, align=None) -> None:
        super().__init__(text, parent)
        self._color, self._size, self._weight = color, size, weight
        self.setFont(font(size, weight))
        self.setAttribute(Qt.WA_TranslucentBackground)
        if align is not None:
            self.setAlignment(align)
        self.restyle()

    def restyle(self) -> None:
        c = self._color
        css = c if (c.startswith("#") or c.startswith("rgba")) else T.css(c)
        self.setStyleSheet(f"color:{css}; background:transparent;")

    def set_color(self, c: str) -> None:
        self._color = c; self.restyle()


# ---------------------------------------------------------------------------
class Button(QAbstractButton, Hoverable):
    """kind: primary | ghost | glass (for use over images) | danger | plain"""
    def __init__(self, text="", kind="primary", icon: str | None = None, parent=None, height=44) -> None:
        super().__init__(parent)
        self._init_hover()
        self.kind, self.icon_name, self._busy, self._spin = kind, icon, False, 0.0
        self.setText(text); self.setCursor(Qt.PointingHandCursor); self.setFixedHeight(height)
        self.setFont(font(11, 700))
        self.setAttribute(Qt.WA_Hover)
        self._timer = QTimer(self); self._timer.timeout.connect(self._tick); self._timer.setInterval(16)

    def sizeHint(self) -> QSize:
        fm = QFontMetrics(self.font())
        w = fm.horizontalAdvance(self.text()) + 40 + (26 if self.icon_name or self._busy else 0)
        return QSize(max(w, 64), self.height())

    def set_busy(self, b: bool) -> None:
        self._busy = b
        self.setEnabled(not b)
        (self._timer.start() if b else self._timer.stop())
        self.update()

    def _tick(self) -> None:
        self._spin = (self._spin + 9) % 360; self.update()

    def enterEvent(self, e): self._hover_to(1); super().enterEvent(e)
    def leaveEvent(self, e): self._hover_to(0); super().leaveEvent(e)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        rad = 13
        down = self.isDown()
        if self.kind == "primary":
            g = QLinearGradient(0, 0, r.width(), r.height())
            a, b = T.c("accent"), T.c("accent2")
            lift = 1 + 0.07 * self._h
            g.setColorAt(0, a.lighter(int(100 * lift))); g.setColorAt(1, b.lighter(int(100 * lift)))
            if T.dark and self._h > 0.01:
                _g = QColor(T.c("accent")); _g.setAlphaF(0.20 * self._h)
                p.setPen(Qt.NoPen); p.setBrush(_g); p.drawRoundedRect(r.adjusted(-2, 1, 2, 3), rad + 2, rad + 2)
            p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); fg = T.c("on_accent")
        elif self.kind == "glass":
            p.setPen(QPen(QColor(255, 255, 255, int(70 + 70 * self._h)), 1)); p.setBrush(QColor(255, 255, 255, int(34 + 40 * self._h)))
            fg = QColor("#FFFFFF")
        elif self.kind == "danger":
            c = T.c("danger"); c.setAlphaF(0.12 + 0.12 * self._h)
            p.setPen(QPen(T.c("danger", 0.5), 1)); p.setBrush(c); fg = T.c("danger")
        else:
            c = T.c("surface_hi") if self.kind == "ghost" else QColor(0, 0, 0, 0)
            if self._h > 0 and self.kind == "plain":
                c = T.c("surface_hi", 0.6 * self._h + 0.01)
            p.setPen(QPen(T.c("border_hi") if self.kind == "ghost" else Qt.NoPen, 1)); p.setBrush(c); fg = T.c("text")
            if self.kind == "ghost" and self._h > 0:
                p.setPen(QPen(T.c("accent", 0.35 + 0.65 * self._h), 1))
        if down:
            p.translate(0, 1)
        if not self.isEnabled() and not self._busy:
            p.setOpacity(0.45)
        p.drawRoundedRect(r, rad, rad)
        p.setPen(fg)
        p.setFont(self.font())
        fm = QFontMetrics(self.font())
        tw = fm.horizontalAdvance(self.text())
        has_icon = bool(self.icon_name) or self._busy
        total = tw + (22 if has_icon and self.text() else (18 if has_icon else 0))
        x0 = (self.width() - total) / 2
        rtl = self.isRightToLeft()
        icon_rect = QRectF(0, (self.height() - 18) / 2, 18, 18)
        text_rect = QRectF(0, 0, tw, self.height())
        if rtl:
            icon_rect.moveLeft(x0 + tw + (4 if self.text() else 0)); text_rect.moveLeft(x0)
        else:
            icon_rect.moveLeft(x0); text_rect.moveLeft(x0 + (22 if has_icon and self.text() else (18 if has_icon else 0)))
        if self._busy:
            pen = QPen(fg, 2.2); pen.setCapStyle(Qt.RoundCap); p.setPen(pen)
            p.drawArc(icon_rect.adjusted(2, 2, -2, -2), int(-self._spin * 16), 100 * 16)
            p.setPen(fg)
        elif self.icon_name:
            icons.draw(p, self.icon_name, icon_rect, fg, 1.9)
        if self.text():
            p.drawText(text_rect, Qt.AlignCenter, self.text())


class IconButton(QAbstractButton, Hoverable):
    def __init__(self, icon: str, size=40, tip="", parent=None, radius=12, color="text2", glyph=18) -> None:
        super().__init__(parent)
        self._init_hover()
        self.icon_name, self.radius, self.color_name, self.glyph = icon, radius, color, glyph
        self.setFixedSize(size, size); self.setCursor(Qt.PointingHandCursor); self.setAttribute(Qt.WA_Hover)
        if tip: self.setToolTip(tip)
        self.danger = False
        self.fixed_color: QColor | None = None

    def set_icon(self, name: str) -> None:
        self.icon_name = name; self.update()

    def enterEvent(self, e): self._hover_to(1); super().enterEvent(e)
    def leaveEvent(self, e): self._hover_to(0); super().leaveEvent(e)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        if self.danger and self._h > 0:
            p.setPen(Qt.NoPen); p.setBrush(QColor(229, 70, 60, int(235 * self._h)))
        else:
            if self.fixed_color is not None:
                p.setPen(Qt.NoPen); p.setBrush(QColor(255, 255, 255, int(40 + 50 * self._h)))
            else:
                p.setPen(QPen(T.c("border_hi", 0.9 * self._h), 1)); p.setBrush(T.c("surface_hi", self._h * 0.9))
        p.drawRoundedRect(r, self.radius, self.radius)
        col = QColor("#FFFFFF") if (self.danger and self._h > .5) else (T.c("text") if self._h > 0.4 else T.c(self.color_name))
        if self.fixed_color is not None:
            col = self.fixed_color
        g = QRectF(0, 0, self.glyph, self.glyph); g.moveCenter(QRectF(self.rect()).center())
        icons.draw(p, self.icon_name, g, col, 1.8)


# ---------------------------------------------------------------------------
class Toggle(QAbstractButton):
    toggled_by_user = Signal(bool)

    def __init__(self, checked=False, parent=None) -> None:
        super().__init__(parent)
        self.setCheckable(True); self.setChecked(checked); self.setFixedSize(50, 28)
        self.setCursor(Qt.PointingHandCursor)
        self._x = 1.0 if checked else 0.0
        self._a: QVariantAnimation | None = None
        self.clicked.connect(self._clicked)

    def _clicked(self) -> None:
        self._go(1.0 if self.isChecked() else 0.0)
        self.toggled_by_user.emit(self.isChecked())

    def set_value(self, v: bool) -> None:
        self.setChecked(v); self._x = 1.0 if v else 0.0; self.update()

    def _go(self, v: float) -> None:
        if self._a: self._a.stop()
        self._a = _anim(v, lambda x: (setattr(self, "_x", float(x)), self.update()), self._x, 200)
        self._a.setEasingCurve(QEasingCurve.OutBack)
        self._a.start()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(1, 1, -1, -1)
        t = max(0.0, min(1.0, self._x))
        off, on = T.c("track"), T.c("accent")
        off = QColor(T.c("text3", 0.45)) if not T.dark else T.c("border_hi")
        mix = QColor(int(off.red() + (on.red() - off.red()) * t), int(off.green() + (on.green() - off.green()) * t),
                     int(off.blue() + (on.blue() - off.blue()) * t), int(off.alpha() + (255 - off.alpha()) * t))
        p.setPen(Qt.NoPen); p.setBrush(mix); p.drawRoundedRect(r, r.height() / 2, r.height() / 2)
        d = r.height() - 6
        rtl = self.isRightToLeft()
        tt = (1 - t) if rtl else t
        x = r.left() + 3 + (r.width() - d - 6) * tt
        p.setBrush(QColor(0, 0, 0, 50)); p.drawEllipse(QRectF(x, r.top() + 3.8, d, d))
        p.setBrush(QColor("#FFFFFF") if (t > .5 or not T.dark) else QColor("#D9E2DD"))
        p.drawEllipse(QRectF(x, r.top() + 3, d, d))


class Segmented(QWidget):
    """Pill segmented control with an animated thumb."""
    changed = Signal(str)

    def __init__(self, items: Sequence[tuple[str, str]], current: str, parent=None, icons_: dict | None = None, height=42, min_seg=0) -> None:
        super().__init__(parent)
        self.items, self.current, self.icons_ = list(items), current, icons_ or {}
        self.setFixedHeight(height); self.setCursor(Qt.PointingHandCursor)
        self.setMouseTracking(True)
        self._pos = float(self._index())
        self._a: QVariantAnimation | None = None
        self._min_seg = min_seg
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Fixed)

    def _index(self) -> int:
        for i, (k, _) in enumerate(self.items):
            if k == self.current: return i
        return 0

    def sizeHint(self) -> QSize:
        fm = QFontMetrics(font(10.5, 700))
        seg = max(self._min_seg, max(fm.horizontalAdvance(t) for _, t in self.items) + 44 + (22 if self.icons_ else 0))
        return QSize(seg * len(self.items) + 8, self.height())

    def set_current(self, key: str, animate=True) -> None:
        if key == self.current: return
        self.current = key
        if self._a: self._a.stop()
        if animate:
            self._a = _anim(float(self._index()), lambda v: (setattr(self, "_pos", float(v)), self.update()), self._pos, 240)
            self._a.start()
        else:
            self._pos = float(self._index()); self.update()

    def _slot(self, i: float) -> float:
        n = len(self.items)
        return (n - 1 - i) if self.isRightToLeft() else i

    def mouseReleaseEvent(self, e) -> None:
        n = len(self.items); w = (self.width() - 8) / n
        i = int((e.position().x() - 4) // w)
        i = n - 1 - i if self.isRightToLeft() else i
        if 0 <= i < n and self.items[i][0] != self.current:
            self.set_current(self.items[i][0]); self.changed.emit(self.current)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        p.setPen(QPen(T.c("border"), 1)); p.setBrush(T.c("surface_hi") if T.dark else QColor(27, 36, 32, 14))
        p.drawRoundedRect(r, r.height() / 2.4, r.height() / 2.4)
        n = len(self.items); w = (self.width() - 8) / n
        th = QRectF(4 + self._slot(self._pos) * w, 4, w, self.height() - 8)
        g = QLinearGradient(th.topLeft(), th.bottomRight())
        g.setColorAt(0, T.c("accent")); g.setColorAt(1, T.c("accent2"))
        p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); p.drawRoundedRect(th, (self.height() - 8) / 2.4, (self.height() - 8) / 2.4)
        p.setFont(font(10.5, 700))
        for i, (k, label) in enumerate(self.items):
            cell = QRectF(4 + self._slot(i) * w, 4, w, self.height() - 8)
            sel = max(0.0, 1 - abs(self._pos - i))
            col = QColor(T.c("on_accent")) if sel > 0.5 else T.c("text2")
            p.setPen(col)
            ic = self.icons_.get(k)
            if ic:
                fm = QFontMetrics(p.font()); tw = fm.horizontalAdvance(label); tot = tw + 24
                x0 = cell.center().x() - tot / 2
                ir = QRectF(x0 + (tw + 6 if self.isRightToLeft() else 0), cell.center().y() - 8, 16, 16)
                tr = QRectF(x0 + (0 if self.isRightToLeft() else 22), cell.top(), tw, cell.height())
                icons.draw(p, ic, ir, col, 1.8); p.drawText(tr, Qt.AlignCenter, label)
            else:
                p.drawText(cell, Qt.AlignCenter, label)


class Stepper(QWidget):
    changed = Signal(int)

    def __init__(self, value=0, lo=-30, hi=30, parent=None) -> None:
        super().__init__(parent)
        self.value, self.lo, self.hi = value, lo, hi
        self.setFixedSize(118, 38); self.setCursor(Qt.PointingHandCursor)

    def set_value(self, v: int) -> None:
        self.value = max(self.lo, min(self.hi, v)); self.update()

    def mouseReleaseEvent(self, e) -> None:
        x = e.position().x(); rtl = self.isRightToLeft()
        left_zone, right_zone = x < 36, x > self.width() - 36
        d = 0
        if left_zone: d = 1 if rtl else -1
        elif right_zone: d = -1 if rtl else 1
        if d:
            self.value = max(self.lo, min(self.hi, self.value + d)); self.update(); self.changed.emit(self.value)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        p.setPen(QPen(T.c("border_hi"), 1)); p.setBrush(T.c("surface_hi")); p.drawRoundedRect(r, 12, 12)
        rtl = self.isRightToLeft()
        for i, nm in enumerate(("minus", "plus")):
            zone = QRectF(0 if (i == 0) != rtl else self.width() - 36, 0, 36, self.height())
            icons.draw(p, nm, QRectF(0, 0, 14, 14).translated(zone.center().x() - 7, zone.center().y() - 7), T.c("accent"), 2.0)
        p.setPen(T.c("text")); p.setFont(font(11, 700))
        txt = f"{self.value:+d}" if self.value else "0"
        p.drawText(QRectF(36, 0, self.width() - 72, self.height()), Qt.AlignCenter, txt)


class MinutesInput(QWidget):
    """Type a number of minutes (validated). Emits changed(int) on every valid edit."""
    changed = Signal(int)

    def __init__(self, value: int = 5, lo: int = 0, hi: int = 600, unit: str = "", parent=None, compact: bool = False) -> None:
        super().__init__(parent)
        from PySide6.QtGui import QIntValidator
        from PySide6.QtWidgets import QHBoxLayout
        self.lo, self.hi = lo, hi
        lay = QHBoxLayout(self); lay.setContentsMargins(0, 0, 0, 0); lay.setSpacing(10)
        self.edit = QLineEdit(str(value)); self.edit.setValidator(QIntValidator(lo, hi, self))
        self.edit.setAlignment(Qt.AlignCenter); self.edit.setFixedSize(64 if compact else 84, 38 if compact else 42); self.edit.setFont(font(11.5 if compact else 12, 700))
        self.edit.textChanged.connect(self._tc); self.edit.editingFinished.connect(self._fin)
        lay.addWidget(self.edit)
        if unit:
            lay.addWidget(Label(unit, 10.5, 600, "text2"))

    def value(self) -> int:
        try:
            return max(self.lo, min(self.hi, int(self.edit.text())))
        except ValueError:
            return self.lo

    def set_value(self, v: int) -> None:
        self.edit.setText(str(v))

    def _tc(self, txt: str) -> None:
        if txt.strip().isdigit():
            self.changed.emit(self.value())

    def _fin(self) -> None:
        self.edit.setText(str(self.value()))


class StatusBadge(QWidget):
    def __init__(self, text="", kind="ok", parent=None) -> None:
        super().__init__(parent)
        self.text, self.kind = text, kind
        self.setFixedHeight(26)

    def set(self, text: str, kind: str) -> None:
        self.text, self.kind = text, kind; self.updateGeometry(); self.update()

    def sizeHint(self) -> QSize:
        return QSize(QFontMetrics(font(9, 700)).horizontalAdvance(self.text) + 34, 26)

    def paintEvent(self, _e) -> None:
        col = {"ok": T.c("ok"), "failed": T.c("danger"), "idle": T.c("text3"), "cached": T.c("accent"), "ready": T.c("emerald") if T.dark else T.c("ok")}.get(self.kind, T.c("text3"))
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        bg = QColor(col); bg.setAlphaF(0.14)
        p.setPen(Qt.NoPen); p.setBrush(bg); p.drawRoundedRect(r, 13, 13)
        p.setBrush(col)
        dx = self.width() - 14 if self.isRightToLeft() else 14
        p.drawEllipse(QPointF(dx, 13), 3.2, 3.2)
        p.setPen(col); p.setFont(font(9, 700))
        tr = QRectF(6, 0, self.width() - 26, 26) if self.isRightToLeft() else QRectF(22, 0, self.width() - 26, 26)
        p.drawText(tr, Qt.AlignCenter, self.text)


# ---------------------------------------------------------------------------
class _Popup(QFrame):
    picked = Signal(object)

    def __init__(self, items, placeholder, no_results, parent=None) -> None:
        super().__init__(parent, Qt.Popup | Qt.FramelessWindowHint | Qt.NoDropShadowWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.items, self.no_results = items, no_results
        lay = QVBoxLayout(self); lay.setContentsMargins(10, 10, 10, 10); lay.setSpacing(8)
        self.edit = QLineEdit(); self.edit.setPlaceholderText(placeholder); self.edit.setFont(font(10.5, 500))
        self.list = QListWidget(); self.list.setFont(font(10.5, 500)); self.list.setVerticalScrollMode(QListWidget.ScrollPerPixel)
        lay.addWidget(self.edit); lay.addWidget(self.list, 1)
        self.edit.textChanged.connect(self._fill)
        self.list.itemClicked.connect(self._click)
        self.edit.returnPressed.connect(lambda: self._click(self.list.currentItem() or self.list.item(0)))
        self.setStyleSheet(T.qss())
        self._fill("")

    def _fill(self, q: str) -> None:
        q = q.strip().lower()
        self.list.clear()
        for label, sub, value, hay in self.items:
            if not q or q in hay:
                it = QListWidgetItem(label + (f"   ·   {sub}" if sub else ""))
                it.setData(Qt.UserRole, value); self.list.addItem(it)
        if not self.list.count():
            it = QListWidgetItem(self.no_results); it.setFlags(Qt.NoItemFlags); self.list.addItem(it)
        elif q:
            self.list.setCurrentRow(0)

    def _click(self, it) -> None:
        if it is None or it.data(Qt.UserRole) is None: return
        self.picked.emit(it.data(Qt.UserRole)); self.close()

    def select_value(self, value) -> None:
        for i in range(self.list.count()):
            if self.list.item(i).data(Qt.UserRole) == value:
                self.list.setCurrentRow(i); self.list.scrollToItem(self.list.item(i), QListWidget.PositionAtCenter); break

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(1, 1, -1, -1)
        p.setPen(QPen(T.c("border_hi"), 1)); p.setBrush(QColor(T.c("glass").red(), T.c("glass").green(), T.c("glass").blue(), 250))
        p.drawRoundedRect(r, 16, 16)


class SearchCombo(QAbstractButton, Hoverable):
    """Custom searchable dropdown. items: (label, sublabel, value, search-haystack)."""
    changed = Signal(object)

    def __init__(self, placeholder="", no_results="", parent=None) -> None:
        super().__init__(parent)
        self._init_hover()
        self.items: list = []
        self.value = None
        self.placeholder, self.no_results = placeholder, no_results
        self.setFixedHeight(46); self.setCursor(Qt.PointingHandCursor); self.setAttribute(Qt.WA_Hover)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        self.clicked.connect(self._open)

    def set_items(self, items: list) -> None:
        self.items = items; self.update()

    def set_value(self, v) -> None:
        self.value = v; self.update()

    def _label(self) -> str:
        for label, sub, value, _ in self.items:
            if value == self.value: return label
        return ""

    def enterEvent(self, e): self._hover_to(1); super().enterEvent(e)
    def leaveEvent(self, e): self._hover_to(0); super().leaveEvent(e)

    def _open(self) -> None:
        pop = _Popup(self.items, self.placeholder, self.no_results, self.window())
        pop.picked.connect(self._picked)
        pop.setFixedWidth(max(self.width(), 280)); pop.setFixedHeight(340)
        gp = self.mapToGlobal(QPoint(0, self.height() + 6))
        screen = self.screen().availableGeometry()
        if gp.y() + 340 > screen.bottom():
            gp = self.mapToGlobal(QPoint(0, -346))
        if self.isRightToLeft():
            gp.setX(self.mapToGlobal(QPoint(self.width(), 0)).x() - pop.width())
        pop.move(gp); pop.show(); pop.edit.setFocus(); pop.select_value(self.value)
        self._pop = pop

    def _picked(self, v) -> None:
        self.value = v; self.update(); self.changed.emit(v)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(.5, .5, -.5, -.5)
        p.setPen(QPen(T.c("accent", 0.4 + 0.6 * self._h) if self._h > 0.05 else T.c("border_hi"), 1))
        p.setBrush(T.c("surface_hi") if T.dark else QColor("#FFFFFF"))
        if not T.dark and self._h < 0.05:
            p.setBrush(QColor(27, 36, 32, 10))
        p.drawRoundedRect(r, 14, 14)
        p.setFont(font(11, 500)); p.setPen(T.c("text"))
        rtl = self.isRightToLeft()
        text = self._label() or self.placeholder
        tr = QRectF(16, 0, self.width() - 56, self.height()) if not rtl else QRectF(40, 0, self.width() - 56, self.height())
        fm = QFontMetrics(p.font())
        p.drawText(tr, (Qt.AlignRight if rtl else Qt.AlignLeft) | Qt.AlignVCenter | Qt.AlignAbsolute, fm.elidedText(text, Qt.ElideRight, int(tr.width())))
        cx = 20 if rtl else self.width() - 20
        icons.draw(p, "chev_down", QRectF(cx - 8, self.height() / 2 - 8, 16, 16), T.c("text2"), 2.0)


# ---------------------------------------------------------------------------
class Toast(QWidget):
    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.text, self.kind = "", "ok"
        self.hide()
        self._fx = QGraphicsOpacityEffect(self); self.setGraphicsEffect(self._fx)
        self._t = QTimer(self); self._t.setSingleShot(True); self._t.timeout.connect(self._out)
        self._a: QPropertyAnimation | None = None

    def show_msg(self, text: str, kind="ok", ms=2600) -> None:
        self.text, self.kind = text, kind
        fm = QFontMetrics(font(10.5, 700))
        self.resize(fm.horizontalAdvance(text) + 76, 48)
        pw = self.parent().width()
        self.move((pw - self.width()) // 2, self.parent().height() - 96)
        self.raise_(); self.show()
        self._anim(1.0); self._t.start(ms)
        self.update()

    def _anim(self, v: float) -> None:
        if self._a: self._a.stop()
        self._a = QPropertyAnimation(self._fx, b"opacity"); self._a.setDuration(220); self._a.setEndValue(v)
        self._a.setEasingCurve(QEasingCurve.OutCubic)
        if v == 0: self._a.finished.connect(self.hide)
        self._a.start()

    def _out(self) -> None:
        self._anim(0.0)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(1, 1, -1, -1)
        col = T.c("ok") if self.kind == "ok" else T.c("accent")
        p.setPen(QPen(T.c("border_hi"), 1))
        bg = T.c("bg1").lighter(115) if T.dark else QColor("#FFFFFF")
        p.setBrush(bg); p.drawRoundedRect(r, 24, 24)
        rtl = self.isRightToLeft()
        cx = r.right() - 28 if rtl else r.left() + 28
        p.setPen(Qt.NoPen); c2 = QColor(col); c2.setAlphaF(.18); p.setBrush(c2); p.drawEllipse(QPointF(cx, r.center().y()), 12, 12)
        icons.draw(p, "check", QRectF(cx - 7, r.center().y() - 7, 14, 14), col, 2.4)
        p.setPen(T.c("text")); p.setFont(font(10.5, 700))
        tr = QRectF(r.left() + 16, 0, r.width() - 64, self.height()) if rtl else QRectF(r.left() + 48, 0, r.width() - 64, self.height())
        p.drawText(tr, Qt.AlignCenter, self.text)


class SlideStack(QWidget):
    """Shows one page at a time with a slide + fade transition."""
    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self.pages: list[QWidget] = []
        self.index = -1
        self._grp: QParallelAnimationGroup | None = None

    def add(self, w: QWidget) -> int:
        w.setParent(self); w.hide(); self.pages.append(w); return len(self.pages) - 1

    def resizeEvent(self, e) -> None:
        for w in self.pages:
            w.setGeometry(self.rect())

    def go(self, i: int, direction: int = 1, animate=True) -> None:
        if i == self.index: return
        old = self.pages[self.index] if self.index >= 0 else None
        new = self.pages[i]; self.index = i
        new.setGeometry(self.rect()); new.show(); new.raise_()
        if self._grp: self._grp.stop()
        if not animate or old is None:
            if old: old.hide()
            return
        dx = 70 * direction * (-1 if self.isRightToLeft() else 1)
        grp = QParallelAnimationGroup(self)
        for w, start, end, o0, o1 in ((new, QPoint(dx, 0), QPoint(0, 0), 0.0, 1.0), (old, QPoint(0, 0), QPoint(-dx, 0), 1.0, 0.0)):
            fx = QGraphicsOpacityEffect(w); w.setGraphicsEffect(fx)
            a = QPropertyAnimation(fx, b"opacity"); a.setStartValue(o0); a.setEndValue(o1); a.setDuration(320); a.setEasingCurve(QEasingCurve.OutCubic)
            b = QPropertyAnimation(w, b"pos"); b.setStartValue(start); b.setEndValue(end); b.setDuration(320); b.setEasingCurve(QEasingCurve.OutCubic)
            grp.addAnimation(a); grp.addAnimation(b)
        def done():
            old.hide(); old.move(0, 0)
            for w in (old, new): w.setGraphicsEffect(None)
        grp.finished.connect(done); self._grp = grp; grp.start()
