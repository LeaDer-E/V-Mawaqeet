#include "theme.h"
#include <QVector>
#include <algorithm>

namespace vm::theme {

static const QMap<QString, QString>& base_dark() {
    static const QMap<QString, QString> d = {
        {QStringLiteral("bg0"), QStringLiteral("#0A110E")},
        {QStringLiteral("bg1"), QStringLiteral("#101C17")},
        {QStringLiteral("bg2"), QStringLiteral("#0D1613")},
        {QStringLiteral("surface"), QStringLiteral("#FFFFFF0B")},
        {QStringLiteral("surface_hi"), QStringLiteral("#FFFFFF16")},
        {QStringLiteral("border"), QStringLiteral("#FFFFFF16")},
        {QStringLiteral("border_hi"), QStringLiteral("#FFFFFF2A")},
        {QStringLiteral("text"), QStringLiteral("#F4EFE3")},
        {QStringLiteral("text2"), QStringLiteral("#93A59D")},
        {QStringLiteral("text3"), QStringLiteral("#5F716A")},
        {QStringLiteral("accent"), QStringLiteral("#E6B85C")},
        {QStringLiteral("accent2"), QStringLiteral("#C99A3E")},
        {QStringLiteral("on_accent"), QStringLiteral("#1B1406")},
        {QStringLiteral("emerald"), QStringLiteral("#2FA37C")},
        {QStringLiteral("danger"), QStringLiteral("#E5736A")},
        {QStringLiteral("ok"), QStringLiteral("#5BC49A")},
        {QStringLiteral("shadow"), QStringLiteral("#00000090")},
        {QStringLiteral("glass"), QStringLiteral("#13201AE6")},
        {QStringLiteral("track"), QStringLiteral("#FFFFFF14")},
    };
    return d;
}

static const QMap<QString, QString>& base_light() {
    static const QMap<QString, QString> l = {
        {QStringLiteral("bg0"), QStringLiteral("#FBF8F1")},
        {QStringLiteral("bg1"), QStringLiteral("#F1EBDD")},
        {QStringLiteral("bg2"), QStringLiteral("#F6F1E6")},
        {QStringLiteral("surface"), QStringLiteral("#FFFFFFFF")},
        {QStringLiteral("surface_hi"), QStringLiteral("#FFFFFFFF")},
        {QStringLiteral("border"), QStringLiteral("#1B24200F")},
        {QStringLiteral("border_hi"), QStringLiteral("#1B242026")},
        {QStringLiteral("text"), QStringLiteral("#1B2420")},
        {QStringLiteral("text2"), QStringLiteral("#65736C")},
        {QStringLiteral("text3"), QStringLiteral("#97A29C")},
        {QStringLiteral("accent"), QStringLiteral("#B8892B")},
        {QStringLiteral("accent2"), QStringLiteral("#9A711F")},
        {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")},
        {QStringLiteral("emerald"), QStringLiteral("#0F5C4A")},
        {QStringLiteral("danger"), QStringLiteral("#C2453B")},
        {QStringLiteral("ok"), QStringLiteral("#18865F")},
        {QStringLiteral("shadow"), QStringLiteral("#3A2E1440")},
        {QStringLiteral("glass"), QStringLiteral("#FFFFFFF2")},
        {QStringLiteral("track"), QStringLiteral("#1B24200F")},
    };
    return l;
}

static QString hsl_hex(double h, double s, double l) {
    float hf = static_cast<float>(std::max(0.0, std::min(0.999, h)));
    float sf = static_cast<float>(std::max(0.0, std::min(1.0, s)));
    float lf = static_cast<float>(std::max(0.0, std::min(1.0, l)));
    return QColor::fromHslF(hf, sf, lf).name().toUpper();
}

PaletteEntry gen_palette(const QString& ar, const QString& en, const QString& accent_hex, std::optional<double> hue) {
    QColor a(accent_hex);
    double ah = (a.hslHueF() >= 0.0f) ? static_cast<double>(a.hslHueF()) : 0.0;
    double sat = std::max(0.35, static_cast<double>(a.hslSaturationF()));
    double bh = hue.has_value() ? (*hue / 360.0) : ah;
    double lum = static_cast<double>(a.lightnessF());

    QMap<QString, QString> dark = {
        {QStringLiteral("bg0"), hsl_hex(bh, 0.42, 0.060)},
        {QStringLiteral("bg1"), hsl_hex(bh, 0.38, 0.130)},
        {QStringLiteral("bg2"), hsl_hex(bh, 0.46, 0.040)},
        {QStringLiteral("accent"), a.name().toUpper()},
        {QStringLiteral("accent2"), hsl_hex(ah, sat, std::max(0.18, lum * 0.78))},
        {QStringLiteral("on_accent"), (lum > 0.45) ? QStringLiteral("#10131A") : QStringLiteral("#FFFFFF")}
    };
    QMap<QString, QString> light = {
        {QStringLiteral("bg0"), hsl_hex(bh, 0.70, 0.975)},
        {QStringLiteral("bg1"), hsl_hex(bh, 0.45, 0.900)},
        {QStringLiteral("bg2"), hsl_hex(bh, 0.58, 0.945)},
        {QStringLiteral("accent"), hsl_hex(ah, sat, std::min(0.42, lum * 0.64))},
        {QStringLiteral("accent2"), hsl_hex(ah, sat, std::min(0.32, lum * 0.48))},
        {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}
    };
    return PaletteEntry{ar, en, dark, light};
}

static void init_palettes(QMap<QString, PaletteEntry>& map, QStringList& order) {
    auto add = [&](const QString& key, const PaletteEntry& entry) {
        map.insert(key, entry);
        order.append(key);
    };

    add(QStringLiteral("gold"), {QStringLiteral("الذهبي الكلاسيكي"), QStringLiteral("Classic Gold"), {}, {}});
    add(QStringLiteral("copper"), {
        QStringLiteral("النحاسي البني"), QStringLiteral("Copper Brown"),
        {{QStringLiteral("bg0"), QStringLiteral("#130E0A")}, {QStringLiteral("bg1"), QStringLiteral("#20170F")}, {QStringLiteral("bg2"), QStringLiteral("#0E0A07")}, {QStringLiteral("accent"), QStringLiteral("#E0A56B")}, {QStringLiteral("accent2"), QStringLiteral("#B97B3C")}, {QStringLiteral("on_accent"), QStringLiteral("#1F1206")}},
        {{QStringLiteral("bg0"), QStringLiteral("#FBF6F0")}, {QStringLiteral("bg1"), QStringLiteral("#F0E4D6")}, {QStringLiteral("bg2"), QStringLiteral("#F7EEE3")}, {QStringLiteral("accent"), QStringLiteral("#A8662A")}, {QStringLiteral("accent2"), QStringLiteral("#874E1C")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });
    add(QStringLiteral("royal"), {
        QStringLiteral("الأزرق الملكي"), QStringLiteral("Royal Blue"),
        {{QStringLiteral("bg0"), QStringLiteral("#080D1C")}, {QStringLiteral("bg1"), QStringLiteral("#0F1A35")}, {QStringLiteral("bg2"), QStringLiteral("#060A16")}, {QStringLiteral("accent"), QStringLiteral("#8FB4FF")}, {QStringLiteral("accent2"), QStringLiteral("#5C86E6")}, {QStringLiteral("on_accent"), QStringLiteral("#061233")}},
        {{QStringLiteral("bg0"), QStringLiteral("#F5F8FF")}, {QStringLiteral("bg1"), QStringLiteral("#E4ECFB")}, {QStringLiteral("bg2"), QStringLiteral("#EEF3FD")}, {QStringLiteral("accent"), QStringLiteral("#2F5FCC")}, {QStringLiteral("accent2"), QStringLiteral("#214AA6")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });
    add(QStringLiteral("amethyst"), {
        QStringLiteral("الأرجواني"), QStringLiteral("Amethyst"),
        {{QStringLiteral("bg0"), QStringLiteral("#0F0A19")}, {QStringLiteral("bg1"), QStringLiteral("#1A1230")}, {QStringLiteral("bg2"), QStringLiteral("#0B0713")}, {QStringLiteral("accent"), QStringLiteral("#C7A2FF")}, {QStringLiteral("accent2"), QStringLiteral("#9A6BE0")}, {QStringLiteral("on_accent"), QStringLiteral("#1A0B33")}},
        {{QStringLiteral("bg0"), QStringLiteral("#FAF6FF")}, {QStringLiteral("bg1"), QStringLiteral("#EBE1F8")}, {QStringLiteral("bg2"), QStringLiteral("#F3ECFB")}, {QStringLiteral("accent"), QStringLiteral("#7C46C9")}, {QStringLiteral("accent2"), QStringLiteral("#5E32A0")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });
    add(QStringLiteral("rose"), {
        QStringLiteral("الوردي الياقوتي"), QStringLiteral("Ruby Rose"),
        {{QStringLiteral("bg0"), QStringLiteral("#170A0F")}, {QStringLiteral("bg1"), QStringLiteral("#2A1219")}, {QStringLiteral("bg2"), QStringLiteral("#11070B")}, {QStringLiteral("accent"), QStringLiteral("#FF9BAE")}, {QStringLiteral("accent2"), QStringLiteral("#E0627C")}, {QStringLiteral("on_accent"), QStringLiteral("#330A14")}},
        {{QStringLiteral("bg0"), QStringLiteral("#FFF7F8")}, {QStringLiteral("bg1"), QStringLiteral("#F8E3E7")}, {QStringLiteral("bg2"), QStringLiteral("#FCEEF0")}, {QStringLiteral("accent"), QStringLiteral("#C23A5A")}, {QStringLiteral("accent2"), QStringLiteral("#9E2B47")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });
    add(QStringLiteral("teal"), {
        QStringLiteral("الفيروزي"), QStringLiteral("Turquoise"),
        {{QStringLiteral("bg0"), QStringLiteral("#061416")}, {QStringLiteral("bg1"), QStringLiteral("#0B2428")}, {QStringLiteral("bg2"), QStringLiteral("#041012")}, {QStringLiteral("accent"), QStringLiteral("#54D6C8")}, {QStringLiteral("accent2"), QStringLiteral("#2AA89B")}, {QStringLiteral("on_accent"), QStringLiteral("#04201D")}},
        {{QStringLiteral("bg0"), QStringLiteral("#F3FAF9")}, {QStringLiteral("bg1"), QStringLiteral("#DDEFEC")}, {QStringLiteral("bg2"), QStringLiteral("#EAF5F3")}, {QStringLiteral("accent"), QStringLiteral("#0F8579")}, {QStringLiteral("accent2"), QStringLiteral("#0B655C")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });
    add(QStringLiteral("forest"), {
        QStringLiteral("الأخضر الزمردي"), QStringLiteral("Emerald Forest"),
        {{QStringLiteral("bg0"), QStringLiteral("#07130C")}, {QStringLiteral("bg1"), QStringLiteral("#0F2418")}, {QStringLiteral("bg2"), QStringLiteral("#050E08")}, {QStringLiteral("accent"), QStringLiteral("#7FD99A")}, {QStringLiteral("accent2"), QStringLiteral("#3FB067")}, {QStringLiteral("on_accent"), QStringLiteral("#06210F")}},
        {{QStringLiteral("bg0"), QStringLiteral("#F4FAF5")}, {QStringLiteral("bg1"), QStringLiteral("#DFF0E3")}, {QStringLiteral("bg2"), QStringLiteral("#EBF6EE")}, {QStringLiteral("accent"), QStringLiteral("#1E7A46")}, {QStringLiteral("accent2"), QStringLiteral("#155C34")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });
    add(QStringLiteral("graphite"), {
        QStringLiteral("الجرافيت الفضي"), QStringLiteral("Silver Graphite"),
        {{QStringLiteral("bg0"), QStringLiteral("#0C0D0F")}, {QStringLiteral("bg1"), QStringLiteral("#17191D")}, {QStringLiteral("bg2"), QStringLiteral("#08090A")}, {QStringLiteral("accent"), QStringLiteral("#CBD3DD")}, {QStringLiteral("accent2"), QStringLiteral("#9AA5B3")}, {QStringLiteral("on_accent"), QStringLiteral("#0C0F14")}},
        {{QStringLiteral("bg0"), QStringLiteral("#F7F8FA")}, {QStringLiteral("bg1"), QStringLiteral("#E6E9EE")}, {QStringLiteral("bg2"), QStringLiteral("#EFF1F4")}, {QStringLiteral("accent"), QStringLiteral("#475467")}, {QStringLiteral("accent2"), QStringLiteral("#344054")}, {QStringLiteral("on_accent"), QStringLiteral("#FFFFFF")}}
    });

    struct GenSpec {
        QString ar;
        QString en;
        QString acc;
        double hue;
    };
    const QVector<GenSpec> gen = {
        {QStringLiteral("غروب برتقالي"), QStringLiteral("Sunset Orange"), QStringLiteral("#FF9A5C"), 20},
        {QStringLiteral("القرمزي"), QStringLiteral("Crimson"), QStringLiteral("#FF6B6B"), 355},
        {QStringLiteral("المرجاني"), QStringLiteral("Coral"), QStringLiteral("#FF8F7A"), 8},
        {QStringLiteral("الخوخي"), QStringLiteral("Peach"), QStringLiteral("#FFB48A"), 24},
        {QStringLiteral("الكرز الفاتح"), QStringLiteral("Sakura"), QStringLiteral("#FFA6D5"), 335},
        {QStringLiteral("الفوشيا"), QStringLiteral("Magenta"), QStringLiteral("#F26BD6"), 320},
        {QStringLiteral("النبيذي"), QStringLiteral("Wine"), QStringLiteral("#E06C8F"), 345},
        {QStringLiteral("اللافندر"), QStringLiteral("Lavender"), QStringLiteral("#B8A8FF"), 262},
        {QStringLiteral("النيلي"), QStringLiteral("Indigo"), QStringLiteral("#7C8CFF"), 235},
        {QStringLiteral("منتصف الليل"), QStringLiteral("Midnight"), QStringLiteral("#A9B8FF"), 225},
        {QStringLiteral("المحيطي"), QStringLiteral("Ocean"), QStringLiteral("#4FB6FF"), 205},
        {QStringLiteral("السماوي"), QStringLiteral("Sky"), QStringLiteral("#5CD0F0"), 195},
        {QStringLiteral("الأكوا"), QStringLiteral("Aqua"), QStringLiteral("#4EE0C8"), 178},
        {QStringLiteral("النعناعي"), QStringLiteral("Mint"), QStringLiteral("#6FE3B0"), 150},
        {QStringLiteral("الليموني"), QStringLiteral("Lime"), QStringLiteral("#B5E65A"), 85},
        {QStringLiteral("الزيتوني"), QStringLiteral("Olive"), QStringLiteral("#C5CC5E"), 70},
        {QStringLiteral("الحمضي"), QStringLiteral("Citrus"), QStringLiteral("#FFD23F"), 50},
        {QStringLiteral("الرملي"), QStringLiteral("Sand"), QStringLiteral("#E8C98A"), 40},
        {QStringLiteral("البرونزي"), QStringLiteral("Bronze"), QStringLiteral("#D9A15B"), 30},
        {QStringLiteral("الموكا"), QStringLiteral("Mocha"), QStringLiteral("#C9A18A"), 25},
        {QStringLiteral("الأردوازي"), QStringLiteral("Slate"), QStringLiteral("#8FB0D0"), 215},
        {QStringLiteral("الأوبسيديان الذهبي"), QStringLiteral("Obsidian Gold"), QStringLiteral("#F0C050"), 0},
        {QStringLiteral("الفجر الوردي"), QStringLiteral("Dawn Pink"), QStringLiteral("#FF9EC4"), 300},
        {QStringLiteral("الفيروز العميق"), QStringLiteral("Deep Cyan"), QStringLiteral("#3FD0E0"), 188},
    };
    for (int i = 0; i < gen.size(); ++i) {
        add(QStringLiteral("g%1").arg(i), gen_palette(gen[i].ar, gen[i].en, gen[i].acc, gen[i].hue));
    }
}

const QMap<QString, PaletteEntry>& PALETTES() {
    static QMap<QString, PaletteEntry> map;
    static QStringList order;
    if (map.isEmpty()) {
        init_palettes(map, order);
    }
    return map;
}

const QStringList& PALETTE_ORDER() {
    static QMap<QString, PaletteEntry> map;
    static QStringList order;
    if (order.isEmpty()) {
        init_palettes(map, order);
    }
    return order;
}

QMap<QString, QString> build_palette(const QString& name, const QString& mode, const QString& custom_accent) {
    QMap<QString, QString> base = (mode == QStringLiteral("light")) ? base_light() : base_dark();
    PaletteEntry entry;
    if (name == QStringLiteral("custom")) {
        entry = gen_palette(QStringLiteral("لونك"), QStringLiteral("Custom"), custom_accent);
    } else {
        const auto& pals = PALETTES();
        entry = pals.contains(name) ? pals.value(name) : pals.value(QStringLiteral("gold"));
    }
    const auto& over = (mode == QStringLiteral("light")) ? entry.light_over : entry.dark_over;
    for (auto it = over.begin(); it != over.end(); ++it) {
        base[it.key()] = it.value();
    }
    if (mode != QStringLiteral("light") && !over.isEmpty()) {
        base[QStringLiteral("glass")] = base[QStringLiteral("bg1")] + QStringLiteral("E6");
    }
    return base;
}

static QColor parse_qc(const QString& h) {
    if (h.length() == 9 && h.startsWith(QLatin1Char('#'))) {
        bool ok = false;
        int r = h.mid(1, 2).toInt(&ok, 16);
        int g = h.mid(3, 2).toInt(&ok, 16);
        int b = h.mid(5, 2).toInt(&ok, 16);
        int a = h.mid(7, 2).toInt(&ok, 16);
        return QColor(r, g, b, a);
    }
    return QColor(h);
}

Theme::Theme() : m_p(base_dark()) {}

void Theme::set_mode(const QString& m, const QString& pal) {
    mode = (m == QStringLiteral("light")) ? QStringLiteral("light") : QStringLiteral("dark");
    palette = (PALETTES().contains(pal) || pal == QStringLiteral("custom")) ? pal : QStringLiteral("gold");
    m_p = build_palette(palette, mode, custom_accent);
}

QMap<QString, QColor> Theme::preview(const QString& pal, const QString& m) const {
    auto p = build_palette(pal, m, custom_accent);
    QMap<QString, QColor> out;
    for (auto it = p.begin(); it != p.end(); ++it) {
        out[it.key()] = parse_qc(it.value());
    }
    return out;
}

QColor Theme::c(const QString& name, std::optional<double> alpha) const {
    QColor col = (name == QStringLiteral("accent_hi"))
                     ? parse_qc(m_p.value(QStringLiteral("accent"))).lighter(135)
                     : parse_qc(m_p.value(name));
    if (alpha.has_value()) {
        col.setAlphaF(static_cast<float>(*alpha));
    }
    return col;
}

QColor Theme::on_color(const QColor& bg) {
    return (bg.lightnessF() > 0.58f) ? QColor(QStringLiteral("#10131A")) : QColor(QStringLiteral("#FFFFFF"));
}

QString Theme::css(const QString& name) const {
    QColor col = c(name);
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(col.red())
        .arg(col.green())
        .arg(col.blue())
        .arg(col.alphaF(), 0, 'f', 3);
}

QString Theme::qss() const {
    return QStringLiteral(R"(
        QToolTip { background:%1; color:%2; border:1px solid %3;
                   padding:6px 10px; border-radius:8px; }
        QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; border: none; }
        QScrollBar:vertical { background: transparent; width: 10px; margin: 4px 2px; }
        QScrollBar::handle:vertical { background: %3; border-radius: 3px; min-height: 40px; }
        QScrollBar::handle:vertical:hover { background: %4; }
        QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px 4px; }
        QScrollBar::handle:horizontal { background: %3; border-radius: 3px; min-width: 40px; }
        QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page { background: none; border: none; width:0; height:0; }
        QLineEdit { background: %5; color:%2; border:1px solid %3;
                    border-radius: 12px; padding: 9px 14px; selection-background-color:%6;
                    selection-color:%7; }
        QLineEdit:focus { border:1px solid %6; }
        QLineEdit:disabled { color:%4; background: transparent; border:1px dashed %8; }
        QListWidget { background: transparent; border:none; outline:none; color:%2; }
        QListWidget::item { padding: 9px 12px; border-radius: 10px; margin: 1px 0; }
        QListWidget::item:hover { background: %5; }
        QListWidget::item:selected { background: %6; color:%7; }
    )").arg(
        css(QStringLiteral("glass")),
        css(QStringLiteral("text")),
        css(QStringLiteral("border_hi")),
        css(QStringLiteral("text3")),
        css(QStringLiteral("surface_hi")),
        css(QStringLiteral("accent")),
        css(QStringLiteral("on_accent")),
        css(QStringLiteral("border"))
    );
}

Theme& T() {
    static Theme inst;
    return inst;
}

} // namespace vm::theme
