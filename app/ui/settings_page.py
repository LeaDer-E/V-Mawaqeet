"""Settings: location, notifications, sources & calculation, appearance, system."""
from __future__ import annotations
from pathlib import Path

from PySide6.QtCore import Qt, Signal, QTimer
from PySide6.QtWidgets import (QFileDialog, QGridLayout, QHBoxLayout, QScrollArea, QVBoxLayout, QWidget, QFrame)

from .. import autostart, i18n, locations as L
from ..config import PRAYER_KEYS
from ..i18n import t, tp
from ..methods import METHODS, ORDER, default_for
from ..state import AppState
from ..theme import T
from .widgets import (Button, Card, IconButton, Label, MinutesInput, Segmented, SearchCombo, StatusBadge, Stepper, Toggle, font)


class Row(QWidget):
    def __init__(self, label: str, control: QWidget, sub: str = "", stack=False) -> None:
        super().__init__()
        lay = (QVBoxLayout if stack else QHBoxLayout)(self)
        lay.setContentsMargins(0, 0, 0, 0); lay.setSpacing(8 if stack else 14)
        col = QVBoxLayout(); col.setSpacing(0)
        self.l = Label(label, 11, 600, "text"); col.addWidget(self.l)
        if sub:
            col.addWidget(Label(sub, 9.5, 500, "text2"))
        lay.addLayout(col, 1 if not stack else 0)
        lay.addWidget(control, 0 if not stack else 1)
        if not stack:
            lay.setAlignment(control, Qt.AlignVCenter)


class Section(Card):
    def __init__(self, title: str, icon_hint: str = "") -> None:
        super().__init__(24)
        self.body = QVBoxLayout(self); self.body.setContentsMargins(26, 22, 26, 24); self.body.setSpacing(16)
        self.title = Label(title, 14, 800, "accent"); self.body.addWidget(self.title)

    def add(self, w) -> None:
        self.body.addWidget(w) if isinstance(w, QWidget) else self.body.addLayout(w)


class SettingsPage(QWidget):
    languageChanged = Signal(str)
    themeChanged = Signal(str)
    previewSound = Signal(str)
    openThemes = Signal()

    def __init__(self, state: AppState, notifier, locator, parent=None) -> None:
        super().__init__(parent)
        self.state, self.notifier, self.locator = state, notifier, locator
        self.s = state.s
        outer = QVBoxLayout(self); outer.setContentsMargins(30, 14, 30, 0); outer.setSpacing(14)
        self.heading = Label(t("settings"), 27, 800, "text"); outer.addWidget(self.heading)
        self.scroll = QScrollArea(); self.scroll.setWidgetResizable(True); self.scroll.setFrameShape(QFrame.NoFrame)
        self.host = QWidget(); self.grid = QGridLayout(self.host); self.grid.setContentsMargins(0, 4, 8, 28)
        self.grid.setHorizontalSpacing(18); self.grid.setVerticalSpacing(18)
        self.scroll.setWidget(self.host); outer.addWidget(self.scroll, 1)
        self.sections: list[Section] = [self._location(), self._alerts(), self._prayers(), self._sources(), self._look(), self._system()]
        self._cols = 0
        self._flow()
        state.timesChanged.connect(self._update_status)
        self._update_status()

    # ---- layout -----------------------------------------------------------
    def resizeEvent(self, e) -> None:
        self._flow(); super().resizeEvent(e)

    def _flow(self) -> None:
        cols = 2 if self.width() >= 1040 else 1
        if cols == self._cols:
            return
        self._cols = cols
        while self.grid.count():
            self.grid.takeAt(0)
        for lay in getattr(self, "_col_layouts", []):
            lay.setParent(None)
        self._col_layouts = []
        loc, alerts, prayers, src, look, system = self.sections
        groups = [[loc, alerts], [prayers, src, look, system]] if cols == 2 else [self.sections]
        for ci, grp in enumerate(groups):
            col = QVBoxLayout(); col.setSpacing(18)
            for sec in grp:
                col.addWidget(sec)
            col.addStretch(1)
            self._col_layouts.append(col)
            self.grid.addLayout(col, 0, ci)
            self.grid.setColumnStretch(ci, 1)
        for c in range(len(groups), 2):
            self.grid.setColumnStretch(c, 0)

    def _save(self) -> None:
        self.s.save()

    # ---- sections ---------------------------------------------------------
    def _location(self) -> Section:
        sec = Section(t("sec_location"))
        self.auto = Toggle(self.s.auto_location); self.auto.toggled_by_user.connect(self._auto_toggled)
        sec.add(Row(t("auto_loc"), self.auto))
        self.detect_btn = Button(t("detect_now"), "ghost", "locate", height=42)
        self.detect_btn.clicked.connect(self._detect)
        sec.add(self.detect_btn)
        self.detect_info = Label("", 10, 600, "accent"); self.detect_info.setWordWrap(True); self.detect_info.hide()
        sec.add(self.detect_info)
        lang = i18n.lang()
        self.country = SearchCombo(t("search"), t("no_results"))
        self.country.set_items([(c.name(lang), c.code, c.code, f"{c.en} {c.ar} {c.code}".lower()) for c in L.countries()])
        self.country.set_value(self.s.country_code); self.country.changed.connect(self._country_changed)
        sec.add(Row(t("country"), self.country, stack=True))
        self.region = SearchCombo(t("search"), t("no_results"))
        self._fill_regions(); self.region.changed.connect(self._region_changed)
        sec.add(Row(t("region"), self.region, stack=True))
        self._set_manual_enabled()
        return sec

    def _fill_regions(self) -> None:
        lang = i18n.lang(); c = L.by_code(self.s.country_code)
        self.region.set_items([(L.region_name(r, lang), "", i, f"{r.en} {r.ar}".lower()) for i, r in enumerate(c.regions)])
        self.region.set_value(self.s.region_index)

    def _set_manual_enabled(self) -> None:
        for w in (self.country, self.region):
            w.setEnabled(not self.s.auto_location); w.setWindowOpacity(1)
        self.detect_btn.setVisible(self.s.auto_location)

    def _auto_toggled(self, v: bool) -> None:
        self.s.auto_location = v; self._save(); self._set_manual_enabled()
        if v:
            self._detect()

    def _detect(self) -> None:
        self.detect_btn.set_busy(True); self.detect_btn.setText(t("detecting"))
        def done(res):
            self.detect_btn.set_busy(False); self.detect_btn.setText(t("detect_now"))
            if not res:
                self.state.toast.emit("refresh_fail", "warn"); return
            c, idx, fix = res
            self.s.lat, self.s.lng = fix.lat, fix.lng
            self.state.set_region(c.code, idx, save=False)
            self.s.lat, self.s.lng = fix.lat, fix.lng       # keep the precise fix
            self._save(); self.country.set_value(c.code); self._fill_regions(); self.state.reload()
            r = c.regions[idx]; lang = i18n.lang(); mark = "\u200f" if i18n.is_rtl() else "\u200e"
            self.detect_info.setText(f"{mark}{t('detected')} {L.region_name(r, lang)} · {c.name(lang)}  ({'GPS' if fix.source == 'gps' else 'IP'} {fix.lat:.2f}, {fix.lng:.2f})")
            self.detect_info.show()
        self.locator.detect(done)

    def _country_changed(self, code: str) -> None:
        self.s.country_code = code; self.s.region_index = 0
        self.state.set_region(code, 0); self._fill_regions()

    def _region_changed(self, idx: int) -> None:
        self.state.set_region(self.s.country_code, idx)

    def _alerts(self) -> Section:
        sec = Section(t("sec_alerts"))
        tg = Toggle(self.s.notify); tg.toggled_by_user.connect(lambda v: (setattr(self.s, "notify", v), self._save(), self.state.alertsChanged.emit()))
        sec.add(Row(t("enable_alerts"), tg))
        self.sn_in = MinutesInput(self.s.snooze_minutes, 1, 720, t("minutes"))
        self.sn_in.changed.connect(lambda v: (setattr(self.s, "snooze_minutes", max(1, v)), self._save()))
        sec.add(Row(t("snooze_default"), self.sn_in))
        se = Toggle(self.s.sound_enabled); se.toggled_by_user.connect(lambda v: (setattr(self.s, "sound_enabled", v), self._save()))
        sec.add(Row(t("sound_enable"), se))
        sec.add(SoundRow(t("adhan_sound"), self.s, "adhan_sound", "adhan", self.previewSound, self._save))
        sec.add(SoundRow(t("pre_sound"), self.s, "pre_sound", "pre", self.previewSound, self._save))
        sec.add(SoundRow(t("iqama_sound"), self.s, "iqama_sound", "iqama", self.previewSound, self._save))
        return sec

    @staticmethod
    def _hdr(title: str, sub: str = "") -> QWidget:
        w = QWidget(); lay = QVBoxLayout(w); lay.setContentsMargins(0, 0, 0, 0); lay.setSpacing(0)
        lay.addWidget(Label(title, 10.5, 700, "accent", align=Qt.AlignCenter))
        if sub:
            lay.addWidget(Label(sub, 8.5, 500, "text2", align=Qt.AlignCenter))
        return w

    @staticmethod
    def _cell(toggle: Toggle, minutes: MinutesInput) -> QWidget:
        w = QWidget(); lay = QHBoxLayout(w); lay.setContentsMargins(0, 0, 0, 0); lay.setSpacing(8)
        lay.addWidget(toggle); lay.addWidget(minutes)
        return w

    def _prayers(self) -> Section:
        """One row per prayer: alarm | reminder (+minutes before) | iqama (+minutes after)."""
        sec = Section(t("sec_prayers"))
        hint = Label(t("prayers_hint2"), 10, 500, "text2"); hint.setWordWrap(True); sec.add(hint)
        g = QGridLayout(); g.setHorizontalSpacing(14); g.setVerticalSpacing(10)
        g.addWidget(QWidget(), 0, 0)
        g.addWidget(self._hdr(t("col_alarm")), 0, 1)
        g.addWidget(self._hdr(t("col_remind"), t("col_before")), 0, 2)
        g.addWidget(self._hdr(t("iqama"), t("col_after")), 0, 3)
        g.setColumnStretch(0, 1)
        self._alarm_t = {}
        for i, k in enumerate(PRAYER_KEYS, start=1):
            g.addWidget(Label(tp(k), 11.5, 700, "text"), i, 0)
            a = Toggle(bool(self.s.alerts.get(k, True))); a.toggled_by_user.connect(lambda v, kk=k: self._set_flag("alerts", kk, v))
            self._alarm_t[k] = a; g.addWidget(a, i, 1, Qt.AlignCenter)
            # reminder before the adhan
            rt = Toggle(bool(self.s.reminders.get(k, False)))
            rm = MinutesInput(int(self.s.reminder_min.get(k, 10)), 1, 240, compact=True); rm.setEnabled(rt.isChecked())
            rt.toggled_by_user.connect(lambda v, kk=k, m=rm: (self._set_flag("reminders", kk, v), m.setEnabled(v)))
            rm.changed.connect(lambda v, kk=k: self._set_minutes("reminder_min", kk, v))
            g.addWidget(self._cell(rt, rm), i, 2, Qt.AlignCenter)
            # iqama after the adhan (not for sunrise)
            if k == "sunrise":
                g.addWidget(Label("—", 12, 700, "text3", align=Qt.AlignCenter), i, 3)
            else:
                it = Toggle(bool(self.s.iqama.get(k, False)))
                im = MinutesInput(int(self.s.iqama_min.get(k, 15)), 1, 120, compact=True); im.setEnabled(it.isChecked())
                it.toggled_by_user.connect(lambda v, kk=k, m=im: (self._set_flag("iqama", kk, v), m.setEnabled(v)))
                im.changed.connect(lambda v, kk=k: self._set_minutes("iqama_min", kk, v))
                g.addWidget(self._cell(it, im), i, 3, Qt.AlignCenter)
        sec.add(g)
        self.state.alertsChanged.connect(self._sync_flags)
        return sec

    def _set_flag(self, which: str, key: str, v: bool) -> None:
        getattr(self.s, which)[key] = v; self._save()
        if which == "alerts":
            self.state.alertsChanged.emit()

    def _set_minutes(self, which: str, key: str, v: int) -> None:
        if v >= 1:
            getattr(self.s, which)[key] = int(v); self._save()

    def _sync_flags(self) -> None:
        for k, tg in self._alarm_t.items():
            tg.set_value(bool(self.s.alerts.get(k, True)))

    def _sources(self) -> Section:
        sec = Section(t("sec_sources"))
        mode = Segmented([("auto", t("src_auto")), ("offline", t("src_offline"))], self.s.source_mode)
        mode.changed.connect(self._mode)
        sec.add(Row(t("primary_src"), mode, stack=True))
        sec.add(Label(t("priority"), 10, 600, "text2"))
        self.badges: dict[str, StatusBadge] = {}
        for i, (k, name) in enumerate((("aladhan", "1 · Aladhan API"), ("aladhan-city", "2 · Aladhan (city)"), ("offline", f"3 · {t('src_local')}"))):
            b = StatusBadge(t("idle"), "idle"); self.badges[k] = b
            sec.add(Row(name, b))
        self.method = SearchCombo(t("search"), t("no_results")); self._fill_methods()
        self.method.changed.connect(self._method_changed)
        sec.add(Row(t("method"), self.method, stack=True))
        sc = Segmented([("0", t("school0")), ("1", t("school1"))], str(self.s.asr_school))
        sc.changed.connect(lambda k: (setattr(self.s, "asr_school", int(k)), self._save(), self.state.reload()))
        sec.add(Row(t("school"), sc, stack=True))
        sec.add(Label(t("adjust"), 10, 600, "text2"))
        g = QGridLayout(); g.setHorizontalSpacing(14); g.setVerticalSpacing(10)
        for i, k in enumerate(PRAYER_KEYS):
            st = Stepper(int(self.s.offsets.get(k, 0))); st.changed.connect(lambda v, kk=k: self._offset(kk, v))
            g.addWidget(Label(tp(k), 10.5, 600, "text"), i // 2, (i % 2) * 2); g.addWidget(st, i // 2, (i % 2) * 2 + 1)
        sec.add(g)
        self.refresh_btn = Button(t("refresh"), "primary", "refresh", height=46)
        self.refresh_btn.clicked.connect(lambda: self.state.reload(force=True))
        sec.add(self.refresh_btn)
        self.state.busyChanged.connect(self.refresh_btn.set_busy)
        return sec

    def _fill_methods(self) -> None:
        lang = i18n.lang(); items = []
        auto_name = METHODS[default_for(self.s.country_code)]
        items.append((("تلقائي — " if lang == "ar" else "Auto — ") + (auto_name.ar if lang == "ar" else auto_name.en), "", "auto", "auto تلقائي"))
        for k in ORDER:
            m = METHODS[k]; items.append((m.ar if lang == "ar" else m.en, "", k, f"{m.en} {m.ar}".lower()))
        self.method.set_items(items); self.method.set_value(self.s.method)

    def _mode(self, k: str) -> None:
        self.s.source_mode = k; self._save(); self.state.reload()

    def _method_changed(self, k: str) -> None:
        self.s.method = k; self._save(); self.state.reload()

    def _offset(self, k: str, v: int) -> None:
        self.s.offsets[k] = v; self._save(); self.state.timesChanged.emit()

    def _update_status(self) -> None:
        st = self.state.statuses
        m = {"ok": ("connected", "ok"), "failed": ("failed", "failed"), "cached": ("cached", "cached"), "ready": ("offline_ready", "ready"),
             "idle": ("idle", "idle"), "skipped": ("idle", "idle")}
        for k, b in self.badges.items():
            v = st.get(k, "idle")
            if k == "offline" and v in ("ready", "ok"):
                v = "ready"
            txt, kind = m.get(v, ("idle", "idle")); b.set(t(txt), kind)

    def _look(self) -> Section:
        sec = Section(t("sec_look"))
        lg = Segmented([("ar", "عربي"), ("en", "English")], i18n.lang()); lg.changed.connect(self.languageChanged.emit)
        sec.add(Row(t("language"), lg, stack=True))
        bt = Button(t("open_themes"), "ghost", "palette", height=44); bt.clicked.connect(self.openThemes.emit)
        sec.add(bt)
        return sec

    def _system(self) -> Section:
        sec = Section(t("sec_system"))
        if autostart.supported():
            a = Toggle(self.s.autostart); a.toggled_by_user.connect(lambda v: (setattr(self.s, "autostart", v), autostart.set_enabled(v), self._save()))
            sec.add(Row(t("autostart"), a))
        tr = Toggle(self.s.minimize_to_tray); tr.toggled_by_user.connect(lambda v: (setattr(self.s, "minimize_to_tray", v), self._save()))
        sec.add(Row(t("tray"), tr))
        return sec


class SoundRow(QWidget):
    """Pick an mp3/wav for one alert type, preview it, or go back to the built-in sound."""
    def __init__(self, label: str, settings, attr: str, kind: str, preview_sig, save) -> None:
        super().__init__()
        self.s, self.attr, self.kind, self.save = settings, attr, kind, save
        col = QVBoxLayout(self); col.setContentsMargins(0, 0, 0, 0); col.setSpacing(8)
        col.addWidget(Label(label, 11, 600, "text"))
        row = QHBoxLayout(); row.setSpacing(8)
        self.btn = Button("", "ghost", "folder", height=42); self.btn.clicked.connect(self._choose)
        self.play = IconButton("play", 42, radius=13); self.play.clicked.connect(lambda: preview_sig.emit(kind))
        self.reset = IconButton("refresh", 42, radius=13, tip=t("reset")); self.reset.clicked.connect(self._reset)
        row.addWidget(self.btn, 1); row.addWidget(self.play); row.addWidget(self.reset)
        col.addLayout(row)
        self._refresh()

    def _refresh(self) -> None:
        path = getattr(self.s, self.attr)
        self.btn.setText(Path(path).name if path else t("default_sound"))
        self.reset.setVisible(bool(path))

    def _choose(self) -> None:
        f, _ = QFileDialog.getOpenFileName(self, t("choose_file"), "", "Audio (*.mp3 *.wav *.ogg *.m4a *.flac *.aac)")
        if f:
            setattr(self.s, self.attr, f); self.save(); self._refresh()

    def _reset(self) -> None:
        setattr(self.s, self.attr, ""); self.save(); self._refresh()
