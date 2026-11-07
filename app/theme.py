"""Design tokens + a tiny observer so painted widgets can follow theme changes."""
from __future__ import annotations
from PySide6.QtGui import QColor

DARK = {
    "bg0": "#0A110E", "bg1": "#101C17", "bg2": "#0D1613",
    "surface": "#FFFFFF0B", "surface_hi": "#FFFFFF16", "border": "#FFFFFF16", "border_hi": "#FFFFFF2A",
    "text": "#F4EFE3", "text2": "#93A59D", "text3": "#5F716A",
    "accent": "#E6B85C", "accent2": "#C99A3E", "on_accent": "#1B1406",
    "emerald": "#2FA37C", "danger": "#E5736A", "ok": "#5BC49A",
    "shadow": "#00000090", "glass": "#13201AE6", "track": "#FFFFFF14",
}
LIGHT = {
    "bg0": "#FBF8F1", "bg1": "#F1EBDD", "bg2": "#F6F1E6",
    "surface": "#FFFFFFFF", "surface_hi": "#FFFFFFFF", "border": "#1B24200F", "border_hi": "#1B242026",
    "text": "#1B2420", "text2": "#65736C", "text3": "#97A29C",
    "accent": "#B8892B", "accent2": "#9A711F", "on_accent": "#FFFFFF",
    "emerald": "#0F5C4A", "danger": "#C2453B", "ok": "#18865F",
    "shadow": "#3A2E1440", "glass": "#FFFFFFF2", "track": "#1B24200F",
}


# name -> (ar, en, dark overrides, light overrides). Keys: bg0 bg1 bg2 accent accent2 on_accent
PALETTES: dict[str, tuple] = {
    "gold": ("الذهبي الكلاسيكي", "Classic Gold", {}, {}),
    "copper": ("النحاسي البني", "Copper Brown",
               dict(bg0="#130E0A", bg1="#20170F", bg2="#0E0A07", accent="#E0A56B", accent2="#B97B3C", on_accent="#1F1206"),
               dict(bg0="#FBF6F0", bg1="#F0E4D6", bg2="#F7EEE3", accent="#A8662A", accent2="#874E1C", on_accent="#FFFFFF")),
    "royal": ("الأزرق الملكي", "Royal Blue",
              dict(bg0="#080D1C", bg1="#0F1A35", bg2="#060A16", accent="#8FB4FF", accent2="#5C86E6", on_accent="#061233"),
              dict(bg0="#F5F8FF", bg1="#E4ECFB", bg2="#EEF3FD", accent="#2F5FCC", accent2="#214AA6", on_accent="#FFFFFF")),
    "amethyst": ("الأرجواني", "Amethyst",
                 dict(bg0="#0F0A19", bg1="#1A1230", bg2="#0B0713", accent="#C7A2FF", accent2="#9A6BE0", on_accent="#1A0B33"),
                 dict(bg0="#FAF6FF", bg1="#EBE1F8", bg2="#F3ECFB", accent="#7C46C9", accent2="#5E32A0", on_accent="#FFFFFF")),
    "rose": ("الوردي الياقوتي", "Ruby Rose",
             dict(bg0="#170A0F", bg1="#2A1219", bg2="#11070B", accent="#FF9BAE", accent2="#E0627C", on_accent="#330A14"),
             dict(bg0="#FFF7F8", bg1="#F8E3E7", bg2="#FCEEF0", accent="#C23A5A", accent2="#9E2B47", on_accent="#FFFFFF")),
    "teal": ("الفيروزي", "Turquoise",
             dict(bg0="#061416", bg1="#0B2428", bg2="#041012", accent="#54D6C8", accent2="#2AA89B", on_accent="#04201D"),
             dict(bg0="#F3FAF9", bg1="#DDEFEC", bg2="#EAF5F3", accent="#0F8579", accent2="#0B655C", on_accent="#FFFFFF")),
    "forest": ("الأخضر الزمردي", "Emerald Forest",
               dict(bg0="#07130C", bg1="#0F2418", bg2="#050E08", accent="#7FD99A", accent2="#3FB067", on_accent="#06210F"),
               dict(bg0="#F4FAF5", bg1="#DFF0E3", bg2="#EBF6EE", accent="#1E7A46", accent2="#155C34", on_accent="#FFFFFF")),
    "graphite": ("الجرافيت الفضي", "Silver Graphite",
                 dict(bg0="#0C0D0F", bg1="#17191D", bg2="#08090A", accent="#CBD3DD", accent2="#9AA5B3", on_accent="#0C0F14"),
                 dict(bg0="#F7F8FA", bg1="#E6E9EE", bg2="#EFF1F4", accent="#475467", accent2="#344054", on_accent="#FFFFFF")),
}


def _hex(h: float, s: float, l: float) -> str:
    return QColor.fromHslF(max(0.0, min(0.999, h)), max(0.0, min(1.0, s)), max(0.0, min(1.0, l))).name().upper()


def gen_palette(ar: str, en: str, accent_hex: str, hue: float | None = None) -> tuple:
    """Build a coherent dark + light palette from one accent colour (and an optional background hue)."""
    a = QColor(accent_hex)
    ah = a.hslHueF() if a.hslHueF() >= 0 else 0.0
    sat = max(0.35, a.hslSaturationF())
    bh = (hue / 360.0) if hue is not None else ah
    lum = a.lightnessF()
    dark = dict(
        bg0=_hex(bh, .42, .060), bg1=_hex(bh, .38, .130), bg2=_hex(bh, .46, .040),
        accent=a.name().upper(), accent2=_hex(ah, sat, max(.18, lum * .78)),
        on_accent="#10131A" if lum > .45 else "#FFFFFF")
    light = dict(
        bg0=_hex(bh, .70, .975), bg1=_hex(bh, .45, .900), bg2=_hex(bh, .58, .945),
        accent=_hex(ah, sat, min(.42, lum * .64)), accent2=_hex(ah, sat, min(.32, lum * .48)), on_accent="#FFFFFF")
    return (ar, en, dark, light)


# (ar, en, accent colour on dark, background hue in degrees)
_GEN = [
    ("غروب برتقالي", "Sunset Orange", "#FF9A5C", 20), ("القرمزي", "Crimson", "#FF6B6B", 355),
    ("المرجاني", "Coral", "#FF8F7A", 8), ("الخوخي", "Peach", "#FFB48A", 24),
    ("الكرز الفاتح", "Sakura", "#FFA6D5", 335), ("الفوشيا", "Magenta", "#F26BD6", 320),
    ("النبيذي", "Wine", "#E06C8F", 345), ("اللافندر", "Lavender", "#B8A8FF", 262),
    ("النيلي", "Indigo", "#7C8CFF", 235), ("منتصف الليل", "Midnight", "#A9B8FF", 225),
    ("المحيطي", "Ocean", "#4FB6FF", 205), ("السماوي", "Sky", "#5CD0F0", 195),
    ("الأكوا", "Aqua", "#4EE0C8", 178), ("النعناعي", "Mint", "#6FE3B0", 150),
    ("الليموني", "Lime", "#B5E65A", 85), ("الزيتوني", "Olive", "#C5CC5E", 70),
    ("الحمضي", "Citrus", "#FFD23F", 50), ("الرملي", "Sand", "#E8C98A", 40),
    ("البرونزي", "Bronze", "#D9A15B", 30), ("الموكا", "Mocha", "#C9A18A", 25),
    ("الأردوازي", "Slate", "#8FB0D0", 215), ("الأوبسيديان الذهبي", "Obsidian Gold", "#F0C050", 0),
    ("الفجر الوردي", "Dawn Pink", "#FF9EC4", 300), ("الفيروز العميق", "Deep Cyan", "#3FD0E0", 188),
]
for _i, (_ar, _en, _acc, _h) in enumerate(_GEN):
    PALETTES[f"g{_i}"] = gen_palette(_ar, _en, _acc, _h)


def build_palette(name: str, mode: str, custom_accent: str = "#E6B85C") -> dict:
    base = dict(LIGHT if mode == "light" else DARK)
    if name == "custom":
        entry = gen_palette("لونك", "Custom", custom_accent)
    else:
        entry = PALETTES.get(name) or PALETTES["gold"]
    over = entry[3] if mode == "light" else entry[2]
    base.update(over)
    if mode != "light" and over:
        base["glass"] = base["bg1"] + "E6"
    return base


def _qc(h: str) -> QColor:
    # tokens are #RRGGBB or #RRGGBBAA (CSS order) -> Qt wants #AARRGGBB
    if len(h) == 9:
        return QColor(f"#{h[7:9]}{h[1:7]}")
    return QColor(h)


class _Theme:
    def __init__(self) -> None:
        self.mode = "dark"
        self.palette = "gold"
        self.custom_accent = "#E6B85C"
        self._p = DARK
        self._listeners: list = []

    @property
    def dark(self) -> bool:
        return self.mode == "dark"

    def set_mode(self, mode: str, palette: str = "gold") -> None:
        self.mode = "light" if mode == "light" else "dark"
        self.palette = palette if (palette in PALETTES or palette == "custom") else "gold"
        self._p = build_palette(self.palette, self.mode, self.custom_accent)

    def preview(self, palette: str, mode: str) -> dict[str, QColor]:
        """Colors of another palette (for theme preview cards) without switching."""
        p = build_palette(palette, mode, self.custom_accent)
        return {k: _qc(v) for k, v in p.items()}

    def c(self, name: str, alpha: float | None = None) -> QColor:
        col = _qc(self._p["accent"]).lighter(135) if name == "accent_hi" else _qc(self._p[name])
        if alpha is not None:
            col.setAlphaF(alpha)
        return col

    @staticmethod
    def on_color(bg: QColor) -> QColor:
        """Readable text colour for a given background."""
        return QColor("#10131A") if bg.lightnessF() > 0.58 else QColor("#FFFFFF")

    def css(self, name: str) -> str:
        col = self.c(name)
        return f"rgba({col.red()},{col.green()},{col.blue()},{col.alphaF():.3f})"

    def qss(self) -> str:
        return f"""
        QToolTip {{ background:{self.css('glass')}; color:{self.css('text')}; border:1px solid {self.css('border_hi')};
                    padding:6px 10px; border-radius:8px; }}
        QScrollArea, QScrollArea > QWidget > QWidget {{ background: transparent; border: none; }}
        QScrollBar:vertical {{ background: transparent; width: 10px; margin: 4px 2px; }}
        QScrollBar::handle:vertical {{ background: {self.css('border_hi')}; border-radius: 3px; min-height: 40px; }}
        QScrollBar::handle:vertical:hover {{ background: {self.css('text3')}; }}
        QScrollBar:horizontal {{ background: transparent; height: 10px; margin: 2px 4px; }}
        QScrollBar::handle:horizontal {{ background: {self.css('border_hi')}; border-radius: 3px; min-width: 40px; }}
        QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page {{ background: none; border: none; width:0; height:0; }}
        QLineEdit {{ background: {self.css('surface_hi')}; color:{self.css('text')}; border:1px solid {self.css('border_hi')};
                     border-radius: 12px; padding: 9px 14px; selection-background-color:{self.css('accent')};
                     selection-color:{self.css('on_accent')}; }}
        QLineEdit:focus {{ border:1px solid {self.css('accent')}; }}
        QLineEdit:disabled {{ color:{self.css('text3')}; background: transparent; border:1px dashed {self.css('border')}; }}
        QListWidget {{ background: transparent; border:none; outline:none; color:{self.css('text')}; }}
        QListWidget::item {{ padding: 9px 12px; border-radius: 10px; margin: 1px 0; }}
        QListWidget::item:hover {{ background: {self.css('surface_hi')}; }}
        QListWidget::item:selected {{ background: {self.css('accent')}; color:{self.css('on_accent')}; }}
        """


T = _Theme()
