"""Location detection: OS location (Qt Positioning / GPS) then IP geolocation."""
from __future__ import annotations
import logging
from dataclasses import dataclass
import requests

log = logging.getLogger("vm.geo")


@dataclass
class GeoFix:
    lat: float
    lng: float
    country_code: str | None
    source: str


def locate_by_ip() -> GeoFix | None:
    """Blocking. Tries several keyless services in order."""
    services = [
        ("https://ipwho.is/", lambda j: j.get("success") and (j["latitude"], j["longitude"], j.get("country_code"))),
        ("http://ip-api.com/json/?fields=status,lat,lon,countryCode", lambda j: j.get("status") == "success" and (j["lat"], j["lon"], j.get("countryCode"))),
        ("https://ipapi.co/json/", lambda j: j.get("latitude") is not None and (j["latitude"], j["longitude"], j.get("country_code"))),
    ]
    for url, pick in services:
        try:
            r = requests.get(url, timeout=5, headers={"User-Agent": "V-Mawaqeet/2.0"})
            r.raise_for_status()
            res = pick(r.json())
            if res:
                return GeoFix(float(res[0]), float(res[1]), res[2], "ip")
        except Exception as e:  # noqa: BLE001
            log.warning("ip geo %s failed: %s", url, e)
    return None


def reverse_country(lat: float, lng: float) -> str | None:
    """Country of the nearest known region (not the nearest country centroid)."""
    from .locations import nearest_global
    return nearest_global(lat, lng)[0].code
