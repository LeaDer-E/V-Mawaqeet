"""Central application state: settings, location, timetable, refresh logic."""
from __future__ import annotations
import logging
from dataclasses import dataclass
from datetime import date, datetime, timedelta
from zoneinfo import ZoneInfo

from PySide6.QtCore import QObject, Signal

from . import geo, locations as L, providers as P
from .config import PRAYER_KEYS, Settings
from .hijri import to_hijri
from .methods import resolve
from .prayer_calc import calculate
from .workers import run_async

log = logging.getLogger("vm.state")


@dataclass
class Snapshot:
    today: list                     # [(key, datetime)]
    next_key: str
    next_dt: datetime
    prev_dt: datetime
    prev_key: str
    past: set[str]


class AppState(QObject):
    timesChanged = Signal()
    busyChanged = Signal(bool)
    toast = Signal(str, str)        # message-key, kind
    locationChanged = Signal()
    alertsChanged = Signal()

    def __init__(self, settings: Settings) -> None:
        super().__init__()
        self.s = settings
        self.days: dict[str, P.DayTimes] = {}
        self.source = "offline"
        self.statuses: dict[str, str] = {"aladhan": "idle", "offline": "ready"}
        self.busy = False
        self._gen = 0

    # ---- location -------------------------------------------------------
    @property
    def country(self) -> L.Country:
        return L.by_code(self.s.country_code)

    @property
    def region(self) -> L.Region:
        regs = self.country.regions
        return regs[min(max(self.s.region_index, 0), len(regs) - 1)]

    def place(self) -> P.Place:
        c, r = self.country, self.region
        return P.Place(self.s.lat, self.s.lng, self.s.tz or r.tz, r.en, c.en, c.code)

    def set_region(self, code: str, index: int, save=True) -> None:
        c = L.by_code(code)
        index = min(max(index, 0), len(c.regions) - 1)
        r = c.regions[index]
        self.s.country_code, self.s.region_index = c.code, index
        self.s.lat, self.s.lng, self.s.tz = r.lat, r.lng, r.tz
        if save:
            self.s.save()
        self.locationChanged.emit()
        self.reload()

    def tz(self) -> ZoneInfo:
        try:
            return ZoneInfo(self.s.tz)
        except Exception:
            return ZoneInfo("UTC")

    def now(self) -> datetime:
        return P.now_in(self.s.tz)

    def method(self):
        return resolve(self.s.method, self.s.country_code)

    # ---- timetable --------------------------------------------------------
    def _ensure_day(self, d: date) -> P.DayTimes:
        iso = d.isoformat()
        dt = self.days.get(iso)
        if dt is None:   # month missing -> compute locally so we never show nothing
            m = self.method()
            r = calculate(d, self.s.lat, self.s.lng, self.s.tz, m, self.s.asr_school)
            dt = P.DayTimes(d, {k: v.strftime("%H:%M") for k, v in r.items()}, to_hijri(d))
            self.days[iso] = dt
        return dt

    def day_prayers(self, d: date) -> list[tuple[str, datetime]]:
        dt = self._ensure_day(d)
        tz = self.tz()
        out = []
        for k in PRAYER_KEYS:
            h, m = (int(x) for x in dt.times[k].split(":"))
            t = datetime(d.year, d.month, d.day, h, m, tzinfo=tz) + timedelta(minutes=int(self.s.offsets.get(k, 0)))
            out.append((k, t))
        return out

    def hijri(self, d: date) -> tuple[int, int, int]:
        return self._ensure_day(d).hijri or to_hijri(d)

    def snapshot(self) -> Snapshot:
        now = self.now()
        today = self.day_prayers(now.date())
        prev_list = self.day_prayers(now.date() - timedelta(days=1))
        next_list = self.day_prayers(now.date() + timedelta(days=1))
        allp = prev_list + today + next_list
        nxt = next(((k, t) for k, t in allp if t > now), allp[-1])
        prev = max(((t, k) for k, t in allp if t <= now), default=(nxt[1] - timedelta(hours=6), "isha"))
        past = {k for k, t in today if t <= now}
        return Snapshot(today, nxt[0], nxt[1], prev[0], prev[1], past)

    # ---- loading -----------------------------------------------------------
    def _months_needed(self) -> list[tuple[int, int]]:
        n = self.now().date()
        ms = {(d.year, d.month) for d in (n - timedelta(days=1), n, n + timedelta(days=1))}
        return sorted(ms)

    def reload(self, force: bool = False) -> None:
        """Show something instantly (cache or local calc), then refresh online in the background."""
        self._gen += 1
        gen = self._gen
        pl = self.place()
        self.days = {}
        missing: list[tuple[int, int]] = []
        for y, m in self._months_needed():
            res = P.load_cached(pl, self.s, y, m) if not force else None
            if res is None:
                res = P.local_month(pl, self.s, y, m)
                if self.s.source_mode != "offline":
                    missing.append((y, m))
                self.source = "offline"
                self.statuses = res.statuses
            else:
                self.source, self.statuses = res.source, res.statuses
            self.days.update(res.days)
        self.timesChanged.emit()
        if self.s.source_mode == "offline":
            self._set_busy(False)
            if force:
                self.toast.emit("refreshed", "ok")
            return
        if not missing and not force:
            return
        if force:
            missing = self._months_needed()
        self._set_busy(True)
        s_copy = Settings(**{**self.s.__dict__})

        def job():
            return [P.fetch_month(pl, s_copy, y, m, force=force) for y, m in missing]

        def done(results, err):
            if gen != self._gen:
                return
            self._set_busy(False)
            if err or not results:
                self.statuses = {"aladhan": "failed", "offline": "ok"}
                self.toast.emit("refresh_fail", "warn") if force else None
                self.timesChanged.emit()
                return
            online = False
            for r in results:
                self.days.update(r.days)
                self.statuses = r.statuses
                self.source = r.source
                online = online or r.source in ("aladhan", "aladhan-city", "cache")
            self.timesChanged.emit()
            if force or online:
                self.toast.emit("refreshed" if online else "refresh_fail", "ok" if online else "warn")
        run_async(job, done)

    def _set_busy(self, b: bool) -> None:
        if self.busy != b:
            self.busy = b
            self.busyChanged.emit(b)

    # ---- auto location -----------------------------------------------------
    def detect_location(self, on_done) -> None:
        """Async: OS location is tried by the UI layer first; this does the IP path."""
        def done(fix, err):
            if err or not fix:
                on_done(None); return
            on_done(self.apply_fix(fix))
        run_async(geo.locate_by_ip, done)

    def apply_fix(self, fix: "geo.GeoFix"):
        """Map a coordinate fix to (country, region index, fix).

        GPS / OS fixes are precise, so the nearest region over ALL countries decides the country.
        IP fixes carry a country code from the provider; it is trusted only if a region of that
        country is reasonably close, otherwise we fall back to the global nearest region."""
        if fix.source == "ip" and fix.country_code:
            c, idx = L.nearest(fix.country_code, fix.lat, fix.lng)
            r = c.regions[idx]
            if L.haversine(fix.lat, fix.lng, r.lat, r.lng) < 1200:
                return c, idx, fix
        c, idx, _d = L.nearest_global(fix.lat, fix.lng)
        return c, idx, fix
