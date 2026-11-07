"""Tabular (Kuwaiti algorithm) Gregorian -> Hijri conversion, used offline."""
from __future__ import annotations
from datetime import date


def to_hijri(d: date) -> tuple[int, int, int]:
    y, m, dd = d.year, d.month, d.day
    a = (14 - m) // 12
    yy = y + 4800 - a
    mm = m + 12 * a - 3
    jd = dd + (153 * mm + 2) // 5 + 365 * yy + yy // 4 - yy // 100 + yy // 400 - 32045
    l = jd - 1948440 + 10632
    n = (l - 1) // 10631
    l = l - 10631 * n + 354
    j = ((10985 - l) // 5316) * ((50 * l) // 17719) + (l // 5670) * ((43 * l) // 15238)
    l = l - ((30 - j) // 15) * ((17719 * j) // 50) - (j // 16) * ((15238 * j) // 43) + 29
    month = (24 * l) // 709
    day = l - (709 * month) // 24
    year = 30 * n + j - 30
    return day, month, year
