"""Prayer-time engine: cache -> Aladhan (coords) -> Aladhan (city) -> offline calc.

All network access has a timeout and exponential-backoff retry and is meant
to be called from a worker thread (see workers.py), never from the UI thread.
"""
from __future__ import annotations
import json, logging, time
from dataclasses import dataclass, field
from datetime import date, datetime, timedelta
from zoneinfo import ZoneInfo

import requests

from .config import PRAYER_KEYS, Settings
from .hijri import to_hijri
from .methods import resolve
from .paths import cache_dir
from .prayer_calc import calculate

log = logging.getLogger("vm.providers")
ALADHAN = "https://api.aladhan.com/v1"
_MAP = {"Fajr": "fajr", "Sunrise": "sunrise", "Dhuhr": "dhuhr", "Asr": "asr", "Maghrib": "maghrib", "Isha": "isha"}


@dataclass
class DayTimes:
    day: date
    times: dict[str, str]                      # key -> "HH:MM" (location time, before offsets)
    hijri: tuple[int, int, int] | None = None  # (day, month, year)


@dataclass
class MonthResult:
    days: dict[str, DayTimes] = field(default_factory=dict)   # iso date -> DayTimes
    source: str = "offline"                    # aladhan | aladhan-city | cache | offline
    statuses: dict[str, str] = field(default_factory=dict)    # provider -> ok/failed/skipped/cached


@dataclass(frozen=True)
class Place:
    lat: float
    lng: float
    tz: str
    city: str
    country: str
    country_code: str


def _get(url: str, params: dict, tries: int = 2, timeout: float = 6.0) -> dict:
    last: Exception | None = None
    for i in range(tries):
        try:
            r = requests.get(url, params=params, timeout=timeout, headers={"User-Agent": "V-Mawaqeet/2.0"})
            r.raise_for_status()
            j = r.json()
            if j.get("code") != 200:
                raise ValueError(f"API code {j.get('code')}")
            return j
        except Exception as e:  # noqa: BLE001
            last = e
            if i < tries - 1:
                time.sleep(1.0 * (2 ** i))
    raise last  # type: ignore[misc]


def _parse_aladhan(payload: dict) -> dict[str, DayTimes]:
    out: dict[str, DayTimes] = {}
    for item in payload["data"]:
        g = item["date"]["gregorian"]["date"]            # DD-MM-YYYY
        dd, mm, yy = (int(x) for x in g.split("-"))
        d = date(yy, mm, dd)
        times = {_MAP[k]: v.split(" ")[0][:5] for k, v in item["timings"].items() if k in _MAP}
        if len(times) != 6:
            raise ValueError("incomplete timings")
        h = item["date"]["hijri"]
        out[d.isoformat()] = DayTimes(d, times, (int(h["day"]), int(h["month"]["number"]), int(h["year"])))
    return out


def _offline_month(place: Place, s: Settings, year: int, month: int) -> dict[str, DayTimes]:
    m = resolve(s.method, place.country_code)
    out: dict[str, DayTimes] = {}
    d = date(year, month, 1)
    while d.month == month:
        r = calculate(d, place.lat, place.lng, place.tz, m, s.asr_school)
        out[d.isoformat()] = DayTimes(d, {k: v.strftime("%H:%M") for k, v in r.items()}, to_hijri(d))
        d += timedelta(days=1)
    return out


def _cache_path(place: Place, method_id: int, school: int, year: int, month: int):
    return cache_dir() / f"{place.lat:.2f}_{place.lng:.2f}_m{method_id}_s{school}_{year}-{month:02d}.json"


def _load_cache(path) -> dict[str, DayTimes] | None:
    try:
        raw = json.loads(path.read_text("utf-8"))
        return {k: DayTimes(date.fromisoformat(k), v["t"], tuple(v["h"]) if v.get("h") else None) for k, v in raw.items()}
    except Exception:
        return None


def _save_cache(path, days: dict[str, DayTimes]) -> None:
    try:
        path.write_text(json.dumps({k: {"t": v.times, "h": v.hijri} for k, v in days.items()}), "utf-8")
    except Exception:
        log.exception("cache write failed")


def load_cached(place: Place, s: Settings, year: int, month: int) -> MonthResult | None:
    """Instant, no-network lookup (safe on the UI thread)."""
    if s.source_mode == "offline":
        return None
    m = resolve(s.method, place.country_code)
    days = _load_cache(_cache_path(place, m.aladhan_id, s.asr_school, year, month))
    if days:
        return MonthResult(days, "cache", {"aladhan": "cached", "offline": "ready"})
    return None


def local_month(place: Place, s: Settings, year: int, month: int) -> MonthResult:
    return MonthResult(_offline_month(place, s, year, month), "offline",
                       {"aladhan": "skipped" if s.source_mode == "offline" else "idle", "offline": "ok"})


def fetch_month(place: Place, s: Settings, year: int, month: int, force: bool = False) -> MonthResult:
    """Full chain with fallbacks. Blocking — call from a worker thread."""
    m = resolve(s.method, place.country_code)
    if s.source_mode == "offline":
        return local_month(place, s, year, month)
    path = _cache_path(place, m.aladhan_id, s.asr_school, year, month)
    if not force:
        cached = _load_cache(path)
        if cached:
            return MonthResult(cached, "cache", {"aladhan": "cached", "offline": "ready"})
    statuses = {"aladhan": "idle", "aladhan-city": "idle", "offline": "ready"}
    base = {"method": m.aladhan_id, "school": s.asr_school}
    # 1) Aladhan by coordinates (explicit timezone => consistent with our clock)
    try:
        j = _get(f"{ALADHAN}/calendar/{year}/{month}", {**base, "latitude": place.lat, "longitude": place.lng, "timezonestring": place.tz})
        days = _parse_aladhan(j)
        _save_cache(path, days)
        statuses["aladhan"] = "ok"
        return MonthResult(days, "aladhan", statuses)
    except Exception as e:  # noqa: BLE001
        log.warning("aladhan(coords) failed: %s", e)
        statuses["aladhan"] = "failed"
    # 2) Aladhan by city name
    try:
        j = _get(f"{ALADHAN}/calendarByCity/{year}/{month}", {**base, "city": place.city, "country": place.country})
        days = _parse_aladhan(j)
        _save_cache(path, days)
        statuses["aladhan-city"] = "ok"
        return MonthResult(days, "aladhan-city", statuses)
    except Exception as e:  # noqa: BLE001
        log.warning("aladhan(city) failed: %s", e)
        statuses["aladhan-city"] = "failed"
    # 3) offline astronomical calculation — always works
    statuses["offline"] = "ok"
    return MonthResult(_offline_month(place, s, year, month), "offline", statuses)


def now_in(tz: str) -> datetime:
    try:
        return datetime.now(ZoneInfo(tz))
    except Exception:
        return datetime.now().astimezone()
