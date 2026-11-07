"""Offline astronomical prayer-time calculation (the last-resort fallback).

Based on the well-known PrayTimes.org algorithm. Accurate to ~1-2 minutes
for normal latitudes, and degrades gracefully at high latitudes
(angle-based fajr/isha are replaced by a one-seventh-of-the-night rule).
"""
from __future__ import annotations
import math
from datetime import date, datetime, timedelta
from zoneinfo import ZoneInfo

from .methods import Method

_d2r, _r2d = math.pi / 180, 180 / math.pi
sin = lambda d: math.sin(d * _d2r)
cos = lambda d: math.cos(d * _d2r)
tan = lambda d: math.tan(d * _d2r)
asin = lambda x: math.asin(x) * _r2d
acos = lambda x: math.acos(x) * _r2d
atan = lambda x: math.atan(x) * _r2d
atan2 = lambda y, x: math.atan2(y, x) * _r2d
acot = lambda x: math.atan(1 / x) * _r2d
fix = lambda a, m: a - m * math.floor(a / m)


def _julian(y: int, m: int, d: int) -> float:
    if m <= 2:
        y -= 1; m += 12
    a = math.floor(y / 100)
    b = 2 - a + math.floor(a / 4)
    return math.floor(365.25 * (y + 4716)) + math.floor(30.6001 * (m + 1)) + d + b - 1524.5


def _sun(jd: float) -> tuple[float, float]:
    D = jd - 2451545.0
    g = fix(357.529 + 0.98560028 * D, 360)
    q = fix(280.459 + 0.98564736 * D, 360)
    L = fix(q + 1.915 * sin(g) + 0.020 * sin(2 * g), 360)
    e = 23.439 - 0.00000036 * D
    ra = atan2(cos(e) * sin(L), cos(L)) / 15
    eqt = q / 15 - fix(ra, 24)
    return asin(sin(e) * sin(L)), eqt


def _mid_day(jd: float, t: float) -> float:
    return fix(12 - _sun(jd + t)[1], 24)


def _angle_time(jd, lat, angle, t, ccw=False) -> float:
    decl = _sun(jd + t)[0]
    noon = _mid_day(jd, t)
    x = (-sin(angle) - sin(decl) * sin(lat)) / (cos(decl) * cos(lat))
    if x < -1 or x > 1:
        return float("nan")
    v = acos(x) / 15
    return noon + (-v if ccw else v)


def _asr_time(jd, lat, factor, t) -> float:
    decl = _sun(jd + t)[0]
    angle = -acot(factor + tan(abs(lat - decl)))
    return _angle_time(jd, lat, angle, t)


def calculate(d: date, lat: float, lng: float, tz: str, method: Method, school: int = 0) -> dict[str, datetime]:
    zone = ZoneInfo(tz)
    noon_local = datetime(d.year, d.month, d.day, 12, tzinfo=zone)
    off = noon_local.utcoffset().total_seconds() / 3600
    jd = _julian(d.year, d.month, d.day) - lng / (15 * 24)
    t = {"fajr": 5, "sunrise": 6, "dhuhr": 12, "asr": 13, "maghrib": 18, "isha": 18}
    t = {k: v / 24 for k, v in t.items()}
    for _ in range(2):
        r = {
            "fajr": _angle_time(jd, lat, method.fajr, t["fajr"], True),
            "sunrise": _angle_time(jd, lat, 0.833, t["sunrise"], True),
            "dhuhr": _mid_day(jd, t["dhuhr"]),
            "asr": _asr_time(jd, lat, 1 + (1 if school == 1 else 0), t["asr"]),
            "maghrib": _angle_time(jd, lat, 0.833 + method.maghrib_min if method.maghrib_min else 0.833, t["maghrib"]),
        }
        if method.isha_min:
            r["isha"] = r["maghrib"] + method.isha_min / 60
        else:
            r["isha"] = _angle_time(jd, lat, method.isha, t["isha"])
        # high-latitude fallback
        sr, ss = r["sunrise"], r["maghrib"]
        if not (math.isnan(sr) or math.isnan(ss)):
            night = fix(sr - ss, 24)
            if math.isnan(r["fajr"]): r["fajr"] = sr - night / 7
            if math.isnan(r["isha"]): r["isha"] = ss + night / 7
        t = {k: (v if not math.isnan(v) else 12) / 24 for k, v in r.items()}
    shift = off - lng / 15
    out: dict[str, datetime] = {}
    base = datetime(d.year, d.month, d.day, tzinfo=zone)
    for k, v in r.items():
        if math.isnan(v):
            v = 12.0
        v = v + shift
        sec = round(v * 3600 / 60) * 60  # round to nearest minute
        out[k] = base + timedelta(seconds=sec)
    return out
