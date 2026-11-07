"""Country / region database (250 countries, ~4,000 regions, AR + EN names)."""
from __future__ import annotations
import json, math
from dataclasses import dataclass
from functools import lru_cache
from .paths import asset


@dataclass(frozen=True)
class Region:
    en: str
    ar: str
    lat: float
    lng: float
    tz: str


@dataclass(frozen=True)
class Country:
    code: str
    en: str
    ar: str
    lat: float
    lng: float
    tz: str
    regions: tuple[Region, ...]

    def name(self, lang: str) -> str:
        return self.ar if lang == "ar" else self.en


@lru_cache(maxsize=1)
def countries() -> tuple[Country, ...]:
    raw = json.loads(asset("data", "locations.json").read_text("utf-8"))
    out = []
    for c in raw:
        regs = tuple(Region(r[0], r[1], r[2] or c["lat"] or 0.0, r[3] or c["lng"] or 0.0, r[4] or c["tz"]) for r in c["s"])
        out.append(Country(c["c"], c["en"], c["ar"], c["lat"] or 0.0, c["lng"] or 0.0, c["tz"], regs))
    return tuple(out)


def by_code(code: str) -> Country:
    for c in countries():
        if c.code == code.upper():
            return c
    return countries()[0]


def region_name(r: Region, lang: str) -> str:
    return r.ar if lang == "ar" else r.en


def label(c: Country, r: Region, lang: str) -> tuple[str, str]:
    """(city, country) in the given language."""
    return region_name(r, lang), c.name(lang)


def haversine(lat1, lng1, lat2, lng2) -> float:
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp, dl = p2 - p1, math.radians(lng2 - lng1)
    a = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 6371 * 2 * math.asin(math.sqrt(a))


def nearest(code: str, lat: float, lng: float) -> tuple[Country, int]:
    c = by_code(code)
    best = min(range(len(c.regions)), key=lambda i: haversine(lat, lng, c.regions[i].lat, c.regions[i].lng))
    return c, best


def nearest_global(lat: float, lng: float) -> tuple[Country, int, float]:
    """Nearest REGION across every country (never guess the country from its centroid:
    centroids are far from borders, e.g. Suez is closer to Israel's centroid than to Egypt's)."""
    best = (1e9, countries()[0], 0)
    for c in countries():
        for i, r in enumerate(c.regions):
            d = haversine(lat, lng, r.lat, r.lng)
            if d < best[0]:
                best = (d, c, i)
    return best[1], best[2], best[0]
