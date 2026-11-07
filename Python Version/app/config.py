"""Persistent user settings stored as JSON in the user's AppData folder."""
from __future__ import annotations
import json, logging
from dataclasses import dataclass, field, asdict, fields
from .paths import user_dir

log = logging.getLogger("vm.config")
PRAYER_KEYS = ["fajr", "sunrise", "dhuhr", "asr", "maghrib", "isha"]


@dataclass
class Settings:
    first_run: bool = True
    language: str = "ar"             # ar | en
    theme: str = "dark"              # dark | light | auto
    # location
    auto_location: bool = False
    country_code: str = "EG"
    region_index: int = 0            # index inside the country's region list
    lat: float = 31.2
    lng: float = 29.92
    tz: str = "Africa/Cairo"
    # calculation
    method: str = "auto"             # "auto" or a key from methods.METHODS
    asr_school: int = 0              # 0 = Shafi/Maliki/Hanbali, 1 = Hanafi
    offsets: dict = field(default_factory=lambda: {k: 0 for k in PRAYER_KEYS})
    source_mode: str = "auto"        # auto (online -> offline) | offline
    # notifications
    notify: bool = True
    notify_sunrise: bool = False     # legacy (v2.0/2.1): migrated into `alerts`
    alerts: dict = field(default_factory=lambda: {"fajr": True, "sunrise": False, "dhuhr": True, "asr": True, "maghrib": True, "isha": True})
    reminders: dict = field(default_factory=lambda: {k: False for k in PRAYER_KEYS})          # reminder before the adhan, per prayer
    reminder_min: dict = field(default_factory=lambda: {k: 10 for k in PRAYER_KEYS})          # minutes BEFORE the adhan, per prayer
    iqama: dict = field(default_factory=lambda: {k: False for k in PRAYER_KEYS})              # iqama alert, per prayer
    iqama_min: dict = field(default_factory=lambda: {"fajr": 20, "sunrise": 0, "dhuhr": 15, "asr": 15, "maghrib": 10, "isha": 15})  # minutes AFTER the adhan
    iqama_sound: str = ""            # file path, "" = built-in iqama tone
    snooze_minutes: int = 5
    pre_reminder: int = 0            # legacy (<= v2.2): migrated into reminders / reminder_min
    sound_enabled: bool = True
    adhan_sound: str = ""            # file path, "" = built-in chime
    pre_sound: str = ""              # file path, "" = built-in soft reminder
    palette: str = "gold"            # key of theme.PALETTES, or "custom"
    custom_accent: str = "#E6B85C"
    autostart: bool = False
    minimize_to_tray: bool = True

    @classmethod
    def load(cls) -> "Settings":
        p = user_dir() / "config.json"
        try:
            raw = json.loads(p.read_text("utf-8"))
            names = {f.name for f in fields(cls)}
            if raw.get("custom_sound") and not raw.get("adhan_sound"):      # migrate v2.0 settings
                raw["adhan_sound"] = raw["custom_sound"]
            if raw.get("sound_mode") == "silent":
                raw["sound_enabled"] = False
            s = cls(**{k: v for k, v in raw.items() if k in names})
            if "alerts" not in raw and raw.get("notify_sunrise"):
                s.alerts["sunrise"] = True
            if "reminder_min" not in raw:                                 # migrate the old single global reminder
                pre = int(raw.get("pre_reminder", 0) or 0)
                s.reminder_min = {k: (pre or 10) for k in PRAYER_KEYS}
                if "reminders" not in raw or pre == 0:
                    s.reminders = {k: (pre > 0 and k != "sunrise") for k in PRAYER_KEYS}
            for k in PRAYER_KEYS:
                s.offsets.setdefault(k, 0)
                s.alerts.setdefault(k, k != "sunrise")
                s.reminders.setdefault(k, False)
                s.reminder_min.setdefault(k, 10)
                s.iqama.setdefault(k, False)
                s.iqama_min.setdefault(k, {"fajr": 20, "sunrise": 0, "dhuhr": 15, "asr": 15, "maghrib": 10, "isha": 15}[k])
            return s
        except FileNotFoundError:
            return cls()
        except Exception:
            log.exception("config unreadable, using defaults")
            return cls()

    def save(self) -> None:
        try:
            (user_dir() / "config.json").write_text(
                json.dumps(asdict(self), ensure_ascii=False, indent=2), "utf-8")
        except Exception:
            log.exception("could not save config")
