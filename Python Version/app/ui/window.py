"""Frameless main window: custom title bar, hover-expanding sidebar, pages, tray, notifications."""
from __future__ import annotations
import logging
from datetime import datetime, timedelta

from PySide6.QtCore import (QEasingCurve, QPropertyAnimation, QRectF, QSize, Qt, QTimer, QVariantAnimation, Signal, QPoint)
from PySide6.QtGui import QAction, QColor, QGuiApplication, QIcon, QLinearGradient, QPainter, QPen, QBrush, QCursor, QPixmap
from PySide6.QtWidgets import (QApplication, QGraphicsOpacityEffect, QHBoxLayout, QLabel, QMenu, QStackedWidget, QSystemTrayIcon,
                               QVBoxLayout, QWidget, QAbstractButton)

from .. import APP_NAME, i18n
from ..config import PRAYER_KEYS, Settings
from ..i18n import t
from ..notifier import Notifier
from ..sound import SoundPlayer
from ..state import AppState
from ..theme import T
from . import icons
from .home import HomePage
from .locate import Locator
from .notification import NotificationPopup
from .onboarding import Onboarding
from .settings_page import SettingsPage
from .themes_page import ThemesPage
from .about_dialog import AboutDialog
from .widgets import IconButton, Toast, font

log = logging.getLogger("vm.window")
M = 10          # transparent margin used for the shadow and the resize hit-zone
SIDE_W, SIDE_OPEN = 76, 212


class TitleBar(QWidget):
    def __init__(self, win: "MainWindow") -> None:
        super().__init__(win)
        self.win = win
        self.setFixedHeight(40)
        self.setLayoutDirection(Qt.LeftToRight)          # window chrome never flips
        lay = QHBoxLayout(self); lay.setContentsMargins(14, 4, 8, 0); lay.setSpacing(4)
        self.logo = QLabel(); self.logo.setPixmap(icons.logo_pixmap(22)); self.logo.setFixedSize(22, 22)
        self.title = QLabel(APP_NAME); self.title.setFont(font(10, 700))
        lay.addWidget(self.logo); lay.addSpacing(6); lay.addWidget(self.title); lay.addStretch(1)
        self.b_min = IconButton("minus", 34, radius=10, glyph=15); self.b_max = IconButton("square", 34, radius=10, glyph=14)
        self.b_close = IconButton("x", 34, radius=10, glyph=15); self.b_close.danger = True
        self.b_min.clicked.connect(win.showMinimized); self.b_max.clicked.connect(win.toggle_max); self.b_close.clicked.connect(win.close)
        for b in (self.b_min, self.b_max, self.b_close): lay.addWidget(b)
        self.restyle()

    def restyle(self) -> None:
        self.title.setStyleSheet(f"color:{T.css('text2')}; background:transparent;")

    def mousePressEvent(self, e) -> None:
        if e.button() == Qt.LeftButton and not self.win.isMaximized():
            self.win.windowHandle().startSystemMove()

    def mouseDoubleClickEvent(self, e) -> None:
        self.win.toggle_max()


class NavItem(QAbstractButton):
    def __init__(self, icon: str, label: str) -> None:
        super().__init__()
        self.icon, self.label, self.active, self._h = icon, label, False, 0.0
        self.setFixedHeight(50); self.setCursor(Qt.PointingHandCursor); self.setAttribute(Qt.WA_Hover)
        self._a = None

    def enterEvent(self, e): self._go(1); super().enterEvent(e)
    def leaveEvent(self, e): self._go(0); super().leaveEvent(e)

    def _go(self, v) -> None:
        if self._a: self._a.stop()
        a = QVariantAnimation(self); a.setStartValue(self._h); a.setEndValue(v); a.setDuration(160)
        a.valueChanged.connect(lambda x: (setattr(self, "_h", float(x)), self.update())); self._a = a; a.start()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect()).adjusted(10, 2, -10, -2)
        if self.active:
            g = QLinearGradient(r.topLeft(), r.bottomRight()); g.setColorAt(0, T.c("accent")); g.setColorAt(1, T.c("accent2"))
            p.setPen(Qt.NoPen); p.setBrush(QBrush(g)); p.drawRoundedRect(r, 15, 15)
            fg = T.c("on_accent")
        else:
            p.setPen(Qt.NoPen); p.setBrush(T.c("surface_hi", 0.9 * self._h)); p.drawRoundedRect(r, 15, 15)
            fg = T.c("text") if self._h > .4 else T.c("text2")
        rtl = self.isRightToLeft()
        ic = QRectF(r.right() - 41 if rtl else r.left() + 12, r.center().y() - 11, 22, 22)
        icons.draw(p, self.icon, ic, fg, 1.9)
        if self.width() > 120:
            p.setPen(fg); p.setFont(font(11, 700))
            tr = QRectF(r.left() + 12, r.top(), r.width() - 62, r.height()) if rtl else QRectF(r.left() + 52, r.top(), r.width() - 62, r.height())
            p.drawText(tr, (Qt.AlignRight if rtl else Qt.AlignLeft) | Qt.AlignVCenter | Qt.AlignAbsolute, self.label)


class Sidebar(QWidget):
    navigate = Signal(str)

    def __init__(self, parent: QWidget) -> None:
        super().__init__(parent)
        self.setAttribute(Qt.WA_Hover)
        self.expanded = 0.0
        self._a: QVariantAnimation | None = None
        lay = QVBoxLayout(self); lay.setContentsMargins(0, 16, 0, 16); lay.setSpacing(6)
        self.logo = QLabel(); self.logo.setPixmap(icons.logo_pixmap(40)); self.logo.setAlignment(Qt.AlignCenter)
        lay.addWidget(self.logo); lay.addSpacing(22)
        self.items = {"home": NavItem("home", t("home")), "settings": NavItem("gear", t("settings")), "themes": NavItem("palette", t("themes")), "about": NavItem("info", t("about"))}
        for k, it in self.items.items():
            it.clicked.connect(lambda _=False, kk=k: self.navigate.emit(kk)); lay.addWidget(it)
        lay.addStretch(1)
        self.setFixedWidth(SIDE_W)

    def set_active(self, k: str) -> None:
        for kk, it in self.items.items(): it.active = kk == k; it.update()

    def enterEvent(self, e): self._to(1); super().enterEvent(e)
    def leaveEvent(self, e): self._to(0); super().leaveEvent(e)

    def _to(self, v: float) -> None:
        if self._a: self._a.stop()
        a = QVariantAnimation(self); a.setStartValue(self.expanded); a.setEndValue(v); a.setDuration(200); a.setEasingCurve(QEasingCurve.OutCubic)
        a.valueChanged.connect(self._set); self._a = a; a.start()

    def _set(self, v) -> None:
        self.expanded = float(v)
        w = int(SIDE_W + (SIDE_OPEN - SIDE_W) * self.expanded)
        self.setFixedWidth(w)
        win = self.parentWidget()
        if win: win.place_sidebar()

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        r = QRectF(self.rect())
        p.setPen(QPen(T.c("border"), 1)); p.setBrush(T.c("bg2") if T.dark else QColor("#FFFFFF"))
        rtl = self.isRightToLeft()
        p.drawRoundedRect(r.adjusted(-30 if not rtl else 1, 0, -1 if not rtl else 30, 0), 0, 0)
        if self.expanded > 0.02 and T.dark is False:
            pass


class MainWindow(QWidget):
    def __init__(self, settings: Settings, start_hidden: bool = False) -> None:
        super().__init__(None, Qt.FramelessWindowHint | Qt.Window)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.setMouseTracking(True)
        self.setWindowTitle(APP_NAME)
        self.setWindowIcon(QIcon(icons.logo_pixmap(64)))
        self.setMinimumSize(900 + 2 * M, 600 + 2 * M); self.resize(1280, 820)
        self.s = settings
        self.state = AppState(settings)
        self.notifier = Notifier(self.state)
        self.sound = SoundPlayer(self)
        self.popup = NotificationPopup()
        self.locator = Locator(self.state, self)
        self._maximized = False
        self._quitting = False

        root = QVBoxLayout(self); root.setContentsMargins(M, M, M, M); root.setSpacing(0)
        self.titlebar = TitleBar(self); root.addWidget(self.titlebar)
        self.pages = QStackedWidget(); root.addWidget(self.pages, 1)
        self.sidebar = Sidebar(self); self.sidebar.navigate.connect(self._nav)
        self.toast = Toast(self)
        self.home = self.settings_page = self.themes_page = self.onboarding = None
        self.tray = self._make_tray()

        self.state.timesChanged.connect(self._times_changed)
        self.state.toast.connect(self._toast)
        self.notifier.fire.connect(self._on_fire)
        self.popup.stopped.connect(self.sound.stop)
        self.popup.snoozed.connect(lambda k, m: (self.sound.stop(), self.notifier.snooze(k, m)))
        self.popup.openApp.connect(self.bring_front)
        self.clock = QTimer(self); self.clock.setInterval(1000); self.clock.timeout.connect(self._tick); self.clock.start()

        self._sync_friday()
        self.apply_look(rebuild=False)
        self._build_pages()
        if settings.first_run:
            self.show_onboarding()
        else:
            self.state.reload()
            self.show_page("home", animate=False)
        if not start_hidden:
            self.show()

    # ---- look & language ----------------------------------------------------
    def _effective_theme(self) -> str:
        th = self.s.theme
        if th == "auto":
            try:
                return "dark" if QGuiApplication.styleHints().colorScheme() == Qt.ColorScheme.Dark else "light"
            except Exception:
                return "dark"
        return th

    def apply_look(self, rebuild=True) -> None:
        i18n.set_language(self.s.language)
        QApplication.setLayoutDirection(Qt.RightToLeft if i18n.is_rtl() else Qt.LeftToRight)
        T.custom_accent = self.s.custom_accent; T.set_mode(self._effective_theme(), self.s.palette)
        QApplication.instance().setStyleSheet(T.qss())
        self.titlebar.restyle()
        self.titlebar.logo.setPixmap(icons.logo_pixmap(22)); self.setWindowIcon(QIcon(icons.logo_pixmap(64)))
        self.sidebar.logo.setPixmap(icons.logo_pixmap(40))
        if getattr(self, 'tray', None): self.tray.setIcon(QIcon(icons.logo_pixmap(64)))
        self.update()
        if rebuild:
            self._crossfade(self._rebuild)

    def _crossfade(self, fn) -> None:
        snap = self.grab()
        fn()
        ov = QLabel(self); ov.setPixmap(snap); ov.setGeometry(self.rect()); ov.setAttribute(Qt.WA_TransparentForMouseEvents); ov.show(); ov.raise_()
        fx = QGraphicsOpacityEffect(ov); ov.setGraphicsEffect(fx)
        a = QPropertyAnimation(fx, b"opacity", ov); a.setStartValue(1.0); a.setEndValue(0.0); a.setDuration(380); a.setEasingCurve(QEasingCurve.OutCubic)
        a.finished.connect(ov.deleteLater); a.start(); self._ov_anim = a

    def set_language(self, code: str) -> None:
        self.s.language = code; self.s.save(); self.apply_look()

    def set_theme(self, mode: str) -> None:
        self.s.theme = mode; self.s.save(); self.apply_look()

    def _nav(self, name: str) -> None:
        if name == "about":
            self.show_about()
        else:
            self.show_page(name)

    def show_about(self) -> None:
        if getattr(self, "_about", None) is not None:
            try:
                if self._about.isVisible():
                    return
            except RuntimeError:
                pass
        self._about = AboutDialog(self, self.state.snapshot().next_key, lambda msg: (self.toast.show_msg(msg, "ok", 1600), self.toast.raise_()))
        self._about.closed.connect(lambda: setattr(self, "_about", None))
        self._about.open()
        self.toast.raise_()

    def set_palette(self, name: str) -> None:
        self.s.palette = name; self.s.save(); self.apply_look()

    def set_custom_accent(self, hex_: str) -> None:
        self.s.custom_accent, self.s.palette = hex_, "custom"; self.s.save(); self.apply_look()

    def toggle_theme(self) -> None:
        self.set_theme("light" if T.dark else "dark")

    def _rebuild(self) -> None:
        cur = getattr(self, "_cur", "home")
        onboarding = self.pages.currentWidget() is self.onboarding and self.onboarding is not None
        self._build_pages()
        for k, lbl in (("home", "home"), ("settings", "settings"), ("themes", "themes"), ("about", "about")):
            self.sidebar.items[k].label = t(lbl)
        self.tray_menu_refresh()
        if onboarding:
            self.pages.setCurrentWidget(self.onboarding)
        else:
            self.show_page(cur, animate=False)
        self.place_sidebar()

    def _build_pages(self) -> None:
        for w in (self.home, self.settings_page, self.themes_page, self.onboarding):
            if w is not None:
                self.pages.removeWidget(w); w.hide(); w.setParent(None); w.deleteLater()
        self.home = HomePage(self.state, self.notifier)
        self.home.languageChanged.connect(self.set_language); self.home.themeToggle.connect(self.toggle_theme)
        self.home.snoozeCancel.connect(self.notifier.cancel_snooze)
        self.settings_page = SettingsPage(self.state, self.notifier, self.locator)
        self.settings_page.languageChanged.connect(self.set_language); self.settings_page.themeChanged.connect(self.set_theme)
        self.settings_page.previewSound.connect(self.preview_sound)
        self.settings_page.openThemes.connect(lambda: self.show_page("themes"))
        self.themes_page = ThemesPage(self.s)
        self.themes_page.modeChanged.connect(self.set_theme); self.themes_page.paletteChanged.connect(self.set_palette); self.themes_page.customChosen.connect(self.set_custom_accent)
        self.onboarding = Onboarding(self.state, self.locator)
        self.onboarding.languageChanged.connect(self._ob_language); self.onboarding.themeChanged.connect(self._ob_theme)
        self.onboarding.finished.connect(self._ob_done)
        for w in (self.onboarding, self.home, self.settings_page, self.themes_page): self.pages.addWidget(w)
        margin = (0, 0, SIDE_W, 0) if i18n.is_rtl() else (SIDE_W, 0, 0, 0)
        for w in (self.home, self.settings_page, self.themes_page):
            w.setContentsMargins(*margin)

    # ---- onboarding ---------------------------------------------------------
    def show_onboarding(self) -> None:
        self.sidebar.hide(); self.pages.setCurrentWidget(self.onboarding)

    def _ob_language(self, code: str) -> None:
        self.s.language = code
        i18n.set_language(code); QApplication.setLayoutDirection(Qt.RightToLeft if code == "ar" else Qt.LeftToRight)
        ob_theme = self.s.theme
        self._crossfade(lambda: (self._build_pages(), self.pages.setCurrentWidget(self.onboarding), self.onboarding.build(1)))
        self.s.theme = ob_theme

    def _ob_theme(self, mode: str) -> None:
        self.s.theme = mode
        T.custom_accent = self.s.custom_accent; T.set_mode(self._effective_theme(), self.s.palette); QApplication.instance().setStyleSheet(T.qss()); self.titlebar.restyle(); self.update()
        for w in self.findChildren(QWidget): w.update()

    def _ob_done(self) -> None:
        self.s.save(); self.state.reload()
        self.apply_look(rebuild=True)
        self.show_page("home", animate=False)

    # ---- navigation -----------------------------------------------------------
    def show_page(self, name: str, animate=True) -> None:
        w = {"home": self.home, "settings": self.settings_page, "themes": self.themes_page}[name]
        self._cur = name
        self.sidebar.show(); self.place_sidebar(); self.sidebar.raise_(); self.toast.raise_()
        self.sidebar.set_active(name)
        if self.pages.currentWidget() is w:
            return
        self.pages.setCurrentWidget(w)
        if animate:
            fx = QGraphicsOpacityEffect(w); w.setGraphicsEffect(fx)
            a = QPropertyAnimation(fx, b"opacity", w); a.setStartValue(0.0); a.setEndValue(1.0); a.setDuration(260)
            a.finished.connect(lambda: w.setGraphicsEffect(None)); a.start(); self._pa = a

    def place_sidebar(self) -> None:
        if self.sidebar.isHidden():
            return
        m = 0 if self._maximized else M
        top = m + self.titlebar.height()
        h = self.height() - top - m - 1
        w = self.sidebar.width()
        x = (self.width() - m - w - 0) if i18n.is_rtl() else m
        self.sidebar.setGeometry(x, top, w, h)

    def resizeEvent(self, e) -> None:
        self.place_sidebar(); super().resizeEvent(e)

    # ---- data events -----------------------------------------------------------
    def _toast(self, k: str, kind: str) -> None:
        if "|" in k:
            base, prayer = k.split("|", 1)
            self.toast.show_msg(t(base, name=t(prayer)), kind)
        else:
            self.toast.show_msg(t(k), kind)

    def _times_changed(self) -> None:
        self._sync_friday()
        if self.home: self.home.refresh()

    def _sync_friday(self) -> bool:
        """Friday is decided by the date at the selected location. Returns True if it changed."""
        fri = self.state.now().weekday() == 4
        if fri == i18n.is_friday():
            return False
        i18n.set_friday(fri)
        return True

    def _tick(self) -> None:
        if self._sync_friday():                       # midnight passed: Dhuhr <-> Jumu'ah
            if self.home: self.home.refresh()
            self._tray_update()
        self._tray_n = getattr(self, "_tray_n", 0) + 1
        if getattr(self, "tray", None) and (self._menu.isVisible() or self._tray_n % 15 == 0):
            self._tray_update()
        if self.home and self.pages.currentWidget() is self.home and self.isVisible() and not self.isMinimized():
            self.home.tick()

    # ---- notifications ----------------------------------------------------------
    def _on_fire(self, key: str, kind: str, dt: datetime) -> None:
        s = self.s
        minutes, shown = 0, dt
        if kind == "pre":
            minutes = int(s.reminder_min.get(key, 10))
        elif kind == "iqama":
            shown = dt + timedelta(minutes=int(s.iqama_min.get(key, 15)))
        self.popup.present(key, shown.strftime("%H:%M"), kind, minutes, s.snooze_minutes)
        self.sound.play(self.sound.path_for(s, kind), 900 if kind in ("pre", "iqama") else 1800)

    def preview_sound(self, kind: str = "adhan") -> None:
        self.sound.play(self.sound.path_for(self.s, kind), 300)
        QTimer.singleShot(7000, self.sound.stop)

    # ---- tray ---------------------------------------------------------------------
    def _make_tray(self):
        if not QSystemTrayIcon.isSystemTrayAvailable():
            return None
        tr = QSystemTrayIcon(QIcon(icons.logo_pixmap(64)), self)
        tr.setToolTip(APP_NAME)
        tr.activated.connect(lambda r: self.bring_front() if r in (QSystemTrayIcon.Trigger, QSystemTrayIcon.DoubleClick) else None)
        self._menu = QMenu(); self._menu.aboutToShow.connect(self._tray_update)
        try:
            QGuiApplication.styleHints().colorSchemeChanged.connect(lambda *_: self.tray_menu_refresh())
        except Exception:
            pass
        tr.setContextMenu(self._menu); tr.show()
        self.tray = tr; self.tray_menu_refresh()
        return tr

    def _countdown_text(self):
        snap = self.state.snapshot(); now = self.state.now()
        secs = max(0, int((snap.next_dt - now).total_seconds()))
        h, rem = divmod(secs, 3600); m, s = divmod(rem, 60)
        return snap, f"{h:02d}:{m:02d}:{s:02d}" if h else f"{m:02d}:{s:02d}"

    # ---- tray menu helpers
    @staticmethod
    def _menu_fg() -> QColor:
        """Neutral icon colour that matches the OS menu (light icons on a dark menu and vice-versa)."""
        try:
            dark = QGuiApplication.styleHints().colorScheme() == Qt.ColorScheme.Dark
        except Exception:
            dark = True
        return QColor("#E4E6EA") if dark else QColor("#3B4048")

    def _mi(self, name: str, color: QColor | None = None) -> QIcon:
        return QIcon(icons.pixmap(name, 18, color or self._menu_fg()))

    @staticmethod
    def _blank_icon() -> QIcon:
        pm = QPixmap(36, 36); pm.setDevicePixelRatio(2.0); pm.fill(Qt.transparent)
        return QIcon(pm)

    def tray_menu_refresh(self) -> None:
        """(Re)build the tray menu in the current language. EVERY entry has an icon in the same column,
        so nothing is pushed sideways by check marks. Dynamic parts are refreshed by _tray_update()."""
        if not getattr(self, "tray", None):
            return
        m = self._menu; m.clear()
        self._tray_next = m.addAction(self._mi("clock"), ""); self._tray_next.triggered.connect(self.bring_front)
        sub = m.addMenu(t("today_times")); sub.menuAction().setIcon(self._mi("calendar")); self._tray_times = {}
        for k in PRAYER_KEYS:
            act = sub.addAction(self._blank_icon(), ""); act.triggered.connect(self.bring_front); self._tray_times[k] = act
        m.addSeparator()
        a = m.addAction(QIcon(icons.logo_pixmap(36)), t("tray_open")); a.triggered.connect(self.bring_front)
        f = a.font(); f.setBold(True); a.setFont(f)
        for key, icon, label in (("home", "home", "home"), ("settings", "gear", "settings"), ("themes", "palette", "themes"), ("about", "info", "about")):
            act = m.addAction(self._mi(icon), t(label)); act.triggered.connect(lambda _=False, kk=key: self.open_page(kk))
        m.addSeparator()
        self._tray_alerts = m.addAction(self._mi("bell"), ""); self._tray_alerts.triggered.connect(self._tray_toggle_alerts)
        self._tray_snooze = m.addAction(self._mi("snooze"), t("cancel_snooze")); self._tray_snooze.triggered.connect(self.notifier.cancel_snooze)
        self._tray_theme = m.addAction(self._mi("sun"), ""); self._tray_theme.triggered.connect(self.toggle_theme)
        lang = m.addAction(self._mi("globe"), t("switch_lang")); lang.triggered.connect(lambda: self.set_language("en" if self.s.language == "ar" else "ar"))
        m.addSeparator()
        q = m.addAction(self._mi("power"), t("quit")); q.triggered.connect(self.quit_app)
        self._tray_update()

    def _tray_update(self) -> None:
        if not getattr(self, "tray", None) or not hasattr(self, "_tray_next"):
            return
        snap, left = self._countdown_text()
        self._tray_next.setText(f"{t('next_prayer')}:  " + t("tray_next", name=t(snap.next_key), time=snap.next_dt.strftime("%H:%M"), left=left))
        ok_green, accent = QColor("#3DBE8B"), T.c("accent")
        for k, dt in snap.today:
            act = self._tray_times[k]
            act.setText(f"{t(k)}\t{dt.strftime('%H:%M')}")
            if k == snap.next_key:
                act.setIcon(self._mi("clock", accent))
            elif k in snap.past:
                act.setIcon(self._mi("check", ok_green))
            else:
                act.setIcon(self._blank_icon())
        on = bool(self.s.notify)
        self._tray_alerts.setText(t("tray_alerts_on") if on else t("tray_alerts_off"))
        self._tray_alerts.setIcon(self._mi("bell", accent) if on else self._mi("bell_off"))
        self._tray_snooze.setVisible(self.notifier.next_snooze() is not None)
        self._tray_theme.setText(t("to_light") if T.dark else t("to_dark"))
        self._tray_theme.setIcon(self._mi("sun" if T.dark else "moon"))
        self.tray.setToolTip(f"{APP_NAME}\n{t(snap.next_key)} {snap.next_dt.strftime('%H:%M')} · {left}")

    def _tray_toggle_alerts(self) -> None:
        self._tray_set_alerts(not self.s.notify)

    def _tray_set_alerts(self, on: bool) -> None:
        self.s.notify = on; self.s.save(); self.state.alertsChanged.emit(); self._tray_update()

    def open_page(self, name: str) -> None:
        self.bring_front()
        if self.pages.currentWidget() is self.onboarding:
            return
        self.show_about() if name == "about" else self.show_page(name)

    def bring_front(self) -> None:
        self.showNormal() if self.isMinimized() else self.show()
        self.raise_(); self.activateWindow()

    def quit_app(self) -> None:
        self._quitting = True
        self.sound.stop(); self.popup.hide()
        if self.tray: self.tray.hide()
        QApplication.quit()

    def closeEvent(self, e) -> None:
        if self.s.minimize_to_tray and self.tray and not self._quitting:
            e.ignore(); self.hide()
            return
        self.quit_app(); e.accept()

    # ---- frameless window mechanics ---------------------------------------------------
    def toggle_max(self) -> None:
        self.showNormal() if self.isMaximized() else self.showMaximized()

    def changeEvent(self, e) -> None:
        super().changeEvent(e)
        if e.type() == e.Type.WindowStateChange:
            self._maximized = self.isMaximized()
            m = 0 if self._maximized else M
            self.layout().setContentsMargins(m, m, m, m)
            self.titlebar.b_max.set_icon("restore" if self._maximized else "square")
            self.place_sidebar(); self.update()

    def _edges(self, pos) -> Qt.Edges:
        if self._maximized: return Qt.Edges()
        e = Qt.Edges(); r = self.rect(); g = M + 2
        if pos.x() < g: e |= Qt.LeftEdge
        if pos.x() > r.width() - g: e |= Qt.RightEdge
        if pos.y() < g: e |= Qt.TopEdge
        if pos.y() > r.height() - g: e |= Qt.BottomEdge
        return e

    def mouseMoveEvent(self, ev) -> None:
        ed = self._edges(ev.position().toPoint())
        if ed in (Qt.LeftEdge | Qt.TopEdge, Qt.RightEdge | Qt.BottomEdge): c = Qt.SizeFDiagCursor
        elif ed in (Qt.RightEdge | Qt.TopEdge, Qt.LeftEdge | Qt.BottomEdge): c = Qt.SizeBDiagCursor
        elif ed & (Qt.LeftEdge | Qt.RightEdge): c = Qt.SizeHorCursor
        elif ed & (Qt.TopEdge | Qt.BottomEdge): c = Qt.SizeVerCursor
        else: c = Qt.ArrowCursor
        self.setCursor(c)

    def mousePressEvent(self, ev) -> None:
        ed = self._edges(ev.position().toPoint())
        if ev.button() == Qt.LeftButton and ed:
            self.windowHandle().startSystemResize(ed)

    def paintEvent(self, _e) -> None:
        p = QPainter(self); p.setRenderHint(QPainter.Antialiasing)
        m = 0 if self._maximized else M
        r = QRectF(self.rect()).adjusted(m, m, -m, -m)
        rad = 0 if self._maximized else 20
        if not self._maximized:
            for i in range(m):
                p.setPen(Qt.NoPen); p.setBrush(QColor(0, 0, 0, int((34 if T.dark else 22) * (1 - i / m) ** 2)))
                p.drawRoundedRect(r.adjusted(-i, -i + 3, i, i + 3), rad + i, rad + i)
        g = QLinearGradient(r.topLeft(), r.bottomRight()); g.setColorAt(0, T.c("bg0")); g.setColorAt(1, T.c("bg1"))
        p.setPen(QPen(T.c("border_hi"), 1) if not self._maximized else Qt.NoPen); p.setBrush(QBrush(g)); p.drawRoundedRect(r, rad, rad)
        # ambient gold glow in the corner
        from PySide6.QtGui import QRadialGradient
        p.save(); path = __import__("PySide6.QtGui", fromlist=["QPainterPath"]).QPainterPath(); path.addRoundedRect(r, rad, rad); p.setClipPath(path)
        rg = QRadialGradient(r.right() - 120, r.top() + 40, r.width() * 0.55)
        rg.setColorAt(0, T.c("accent", 0.10 if T.dark else 0.12)); rg.setColorAt(1, QColor(0, 0, 0, 0))
        p.fillRect(r, QBrush(rg)); p.restore()
