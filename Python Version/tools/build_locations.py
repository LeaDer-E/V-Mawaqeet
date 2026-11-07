"""Builds assets/data/locations.json from dr5hn/countries-states-cities-database."""
import json, re, sys
from pathlib import Path
src = Path(sys.argv[1]); out = Path(sys.argv[2])
countries = json.loads((src / "countries.json").read_text("utf-8"))
states = json.loads((src / "states.json").read_text("utf-8"))
SUF = re.compile(r"\s+(Governorate|Province|Region|State|Prefecture|County|District|Department|Municipality|Territory|Emirate|Capital Region)$", re.I)
AR_PRE = re.compile(r"^(محافظة|ولاية|منطقة|إقليم|مقاطعة|إمارة)\s+")
def f(v):
    try: return round(float(v), 3)
    except Exception: return None
by = {}
for s in states:
    if s.get("level") not in (None, "1", 1) and s.get("parent_id"): continue
    lat, lng = f(s.get("latitude")), f(s.get("longitude"))
    ar = (s.get("translations") or {}).get("ar") or s.get("native") or s["name"]
    if not re.search(r"[\u0600-\u06FF]", ar): ar = s["name"]
    by.setdefault(s["country_code"], []).append([SUF.sub("", s["name"]), AR_PRE.sub("", ar), lat, lng, s.get("timezone") or ""])
res = []
for c in countries:
    code = c["iso2"]; tzs = c.get("timezones") or []
    tz0 = tzs[0]["zoneName"] if tzs else "UTC"
    clat, clng = f(c.get("latitude")), f(c.get("longitude"))
    ar = (c.get("translations") or {}).get("ar") or c["name"]
    st = by.get(code, [])
    st = [[a, b, la if la is not None else clat, ln if ln is not None else clng, tz or tz0] for a, b, la, ln, tz in st]
    if not st:
        cap = c.get("capital") or c["name"]
        st = [[cap, cap, clat, clng, tz0]]
    st.sort(key=lambda x: x[0])
    res.append({"c": code, "en": c["name"], "ar": ar, "lat": clat, "lng": clng, "tz": tz0, "s": st})
res.sort(key=lambda x: x["en"])
out.write_text(json.dumps(res, ensure_ascii=False, separators=(",", ":")), "utf-8")
print(len(res), "countries,", sum(len(r["s"]) for r in res), "regions,", out.stat().st_size // 1024, "KB")
