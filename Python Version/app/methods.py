"""Calculation methods: Aladhan id, offline angles, and per-country defaults."""
from __future__ import annotations
from dataclasses import dataclass


@dataclass(frozen=True)
class Method:
    key: str
    aladhan_id: int
    ar: str
    en: str
    fajr: float          # degrees below horizon
    isha: float          # degrees, or 0 when isha_min is used
    isha_min: int = 0    # minutes after maghrib (Umm al-Qura style)
    maghrib_min: float = 0.0


_M = [
    Method("egypt", 5, "الهيئة المصرية العامة للمساحة", "Egyptian General Authority of Survey", 19.5, 17.5),
    Method("ummalqura", 4, "جامعة أم القرى، مكة المكرمة", "Umm Al-Qura University, Makkah", 18.5, 0, 90),
    Method("mwl", 3, "رابطة العالم الإسلامي", "Muslim World League", 18, 17),
    Method("isna", 2, "أمريكا الشمالية (ISNA)", "Islamic Society of North America", 15, 15),
    Method("karachi", 1, "جامعة العلوم الإسلامية، كراتشي", "Univ. of Islamic Sciences, Karachi", 18, 18),
    Method("kuwait", 9, "الكويت", "Kuwait", 18, 17.5),
    Method("qatar", 10, "قطر", "Qatar", 18, 0, 90),
    Method("gulf", 8, "منطقة الخليج", "Gulf Region", 19.5, 0, 90),
    Method("dubai", 16, "دبي", "Dubai", 18.2, 18.2),
    Method("jordan", 23, "وزارة الأوقاف الأردنية", "Jordan Ministry of Awqaf", 18, 18),
    Method("turkey", 13, "رئاسة الشؤون الدينية، تركيا (ديانت)", "Diyanet İşleri Başkanlığı, Turkey", 18, 17),
    Method("tehran", 7, "معهد الجيوفيزياء، جامعة طهران", "Institute of Geophysics, Univ. of Tehran", 17.7, 14, 0, 4.5),
    Method("jafari", 0, "الشيعة الإثنا عشرية (ليفا، قم)", "Shia Ithna-Ashari (Leva, Qum)", 16, 14, 0, 4.0),
    Method("singapore", 11, "سنغافورة (MUIS)", "Singapore (MUIS)", 20, 18),
    Method("malaysia", 17, "ماليزيا (JAKIM)", "Malaysia (JAKIM)", 20, 18),
    Method("indonesia", 20, "إندونيسيا (KEMENAG)", "Indonesia (Kemenag)", 20, 18),
    Method("morocco", 21, "المغرب", "Morocco", 19, 17),
    Method("algeria", 19, "الجزائر", "Algeria", 18, 17),
    Method("tunisia", 18, "تونس", "Tunisia", 18, 18),
    Method("france", 12, "اتحاد المنظمات الإسلامية بفرنسا", "Union of Islamic Organisations of France", 12, 12),
    Method("russia", 14, "الإدارة الدينية لمسلمي روسيا", "Spiritual Administration of Muslims of Russia", 16, 15),
    Method("portugal", 22, "البرتغال", "Portugal", 18, 0, 77),
    Method("moonsighting", 15, "لجنة رؤية الهلال", "Moonsighting Committee Worldwide", 18, 18),
]
METHODS: dict[str, Method] = {m.key: m for m in _M}
ORDER = [m.key for m in _M]

_DEFAULTS = {
    "EG": "egypt", "SD": "egypt", "SS": "egypt", "SA": "ummalqura", "AE": "dubai", "KW": "kuwait", "QA": "qatar",
    "BH": "gulf", "OM": "gulf", "YE": "ummalqura", "JO": "jordan", "PS": "jordan", "LB": "mwl", "SY": "mwl",
    "IQ": "mwl", "IR": "tehran", "TR": "turkey", "AZ": "turkey", "PK": "karachi", "IN": "karachi", "BD": "karachi",
    "AF": "karachi", "LK": "karachi", "MY": "malaysia", "BN": "malaysia", "SG": "singapore", "ID": "indonesia",
    "MA": "morocco", "DZ": "algeria", "TN": "tunisia", "LY": "egypt", "MR": "mwl", "FR": "france", "PT": "portugal",
    "RU": "russia", "KZ": "russia", "US": "isna", "CA": "isna", "MX": "isna", "GB": "moonsighting",
}


def default_for(country_code: str) -> str:
    return _DEFAULTS.get(country_code.upper(), "mwl")


def resolve(key: str, country_code: str) -> Method:
    if key == "auto" or key not in METHODS:
        key = default_for(country_code)
    return METHODS[key]
