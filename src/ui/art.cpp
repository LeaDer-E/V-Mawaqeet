#include "art.h"
#include <QtGlobal>
#include <QBrush>
#include <QImage>
#include <QLinearGradient>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QRadialGradient>
#include <QRectF>
#include <QVector>
#include <cmath>
#include <random>

namespace vm::ui::art {

static constexpr double PI = 3.14159265358979323846;
static double g_U = 170.0;

struct Scene {
    QVector<std::pair<double, QString>> sky;
    QString body;
    QString shade;
    QString dome;
    QString dome_shade;
    QString far_col;
    std::pair<QString, QString> ground;
    std::pair<QString, QString> hero;
    bool sil = false;
    QString windows;
};

static const QMap<QString, Scene>& scenes() {
    static const QMap<QString, Scene> m = {
        {QStringLiteral("fajr"), {
            {{0.0, QStringLiteral("#0A1038")}, {0.42, QStringLiteral("#262C74")}, {0.70, QStringLiteral("#5F4D9C")}, {0.88, QStringLiteral("#C9809F")}, {1.0, QStringLiteral("#F2B49C")}},
            QStringLiteral("#0E1128"), QStringLiteral("#0A0C1E"), QStringLiteral("#0E1128"), QStringLiteral("#0A0C1E"), QStringLiteral("#2A2A5E"),
            {QStringLiteral("#0B0D20"), QStringLiteral("#06070F")},
            {QStringLiteral("#232B6B"), QStringLiteral("#6B4F9E")},
            true, QString()
        }},
        {QStringLiteral("sunrise"), {
            {{0.0, QStringLiteral("#3C4C92")}, {0.40, QStringLiteral("#C37C8C")}, {0.70, QStringLiteral("#F0A468")}, {0.90, QStringLiteral("#F8CB7C")}, {1.0, QStringLiteral("#FFE6A6")}},
            QStringLiteral("#2B1D26"), QStringLiteral("#1C121A"), QStringLiteral("#2B1D26"), QStringLiteral("#1C121A"), QStringLiteral("#B5707A"),
            {QStringLiteral("#24161C"), QStringLiteral("#120B10")},
            {QStringLiteral("#9A5B6E"), QStringLiteral("#F0A468")},
            true, QString()
        }},
        {QStringLiteral("dhuhr"), {
            {{0.0, QStringLiteral("#1B78CC")}, {0.55, QStringLiteral("#5FB1EA")}, {1.0, QStringLiteral("#C4E6F7")}},
            QStringLiteral("#F4EAD3"), QStringLiteral("#CDBB96"), QStringLiteral("#EADFC4"), QStringLiteral("#B7C4C9"), QStringLiteral("#A9CFE8"),
            {QStringLiteral("#E6D8B8"), QStringLiteral("#B9A57C")},
            {QStringLiteral("#1B78CC"), QStringLiteral("#7CC0EE")},
            false, QStringLiteral("#6E8FA3")
        }},
        {QStringLiteral("asr"), {
            {{0.0, QStringLiteral("#5F7DA6")}, {0.45, QStringLiteral("#CFA777")}, {0.80, QStringLiteral("#EFCB8E")}, {1.0, QStringLiteral("#F8E0AE")}},
            QStringLiteral("#E9C998"), QStringLiteral("#A97F4A"), QStringLiteral("#DDB46F"), QStringLiteral("#9A7440"), QStringLiteral("#C9A576"),
            {QStringLiteral("#D2B27F"), QStringLiteral("#8E6D3D")},
            {QStringLiteral("#B0864F"), QStringLiteral("#E9C98F")},
            false, QStringLiteral("#7A5A32")
        }},
        {QStringLiteral("maghrib"), {
            {{0.0, QStringLiteral("#271650")}, {0.32, QStringLiteral("#7F2A68")}, {0.60, QStringLiteral("#D9524F")}, {0.82, QStringLiteral("#F59553")}, {1.0, QStringLiteral("#FDC587")}},
            QStringLiteral("#1B0F1E"), QStringLiteral("#12081A"), QStringLiteral("#1B0F1E"), QStringLiteral("#12081A"), QStringLiteral("#6A2A52"),
            {QStringLiteral("#170B18"), QStringLiteral("#0B050E")},
            {QStringLiteral("#8A2D6B"), QStringLiteral("#F0704F")},
            true, QString()
        }},
        {QStringLiteral("isha"), {
            {{0.0, QStringLiteral("#040920")}, {0.55, QStringLiteral("#0D1C52")}, {0.85, QStringLiteral("#1A3478")}, {1.0, QStringLiteral("#27508F")}},
            QStringLiteral("#070B1B"), QStringLiteral("#04060F"), QStringLiteral("#070B1B"), QStringLiteral("#04060F"), QStringLiteral("#14224E"),
            {QStringLiteral("#060A18"), QStringLiteral("#03050C")},
            {QStringLiteral("#0E1D50"), QStringLiteral("#27508F")},
            true, QStringLiteral("#FFC866")
        }}
    };
    return m;
}

std::pair<QColor, QColor> hero_colors(const QString& key) {
    const auto& sc = scenes().value(key, scenes().value(QStringLiteral("fajr")));
    return {QColor(sc.hero.first), QColor(sc.hero.second)};
}

static QRadialGradient make_rg(double cx, double cy, double r, const QVector<std::pair<double, QColor>>& stops) {
    QRadialGradient g(cx, cy, r);
    for (const auto& [pos, col] : stops) {
        g.setColorAt(pos, col);
    }
    return g;
}

static void draw_glow(QPainter* p, double cx, double cy, double r, const QColor& color, double strength = 0.9) {
    QColor c0(color); c0.setAlphaF(static_cast<float>(strength));
    QColor c1(color); c1.setAlphaF(static_cast<float>(strength * 0.25));
    QColor c2(color); c2.setAlphaF(0.0f);
    p->setPen(Qt::NoPen);
    p->setBrush(QBrush(make_rg(cx, cy, r, {{0.0, c0}, {0.35, c1}, {1.0, c2}})));
    p->drawEllipse(QPointF(cx, cy), r, r);
}

static void draw_stars(QPainter* p, double w, double h, int n, unsigned int seed, double ymax = 0.6, double bright = 1.0) {
    std::mt19937 rnd(seed);
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    const double radii[] = {0.6, 0.7, 0.9, 1.1, 1.5};
    p->setPen(Qt::NoPen);
    for (int i = 0; i < n; ++i) {
        double x = u01(rnd) * w;
        double y = u01(rnd) * h * ymax;
        double fade = 1.0 - std::pow(y / (h * ymax), 1.6);
        double r = radii[rnd() % 5] * (g_U / 170.0);
        double a = std::max(0.0, std::min(1.0, (0.35 + u01(rnd) * 0.65) * fade * bright));
        QColor col(255, 250, 235, static_cast<int>(255 * a));
        p->setBrush(col);
        p->drawEllipse(QPointF(x, y), r, r);
        if (r > 1.3 * (g_U / 170.0) && a > 0.5) {
            p->setPen(QPen(QColor(255, 250, 235, static_cast<int>(120 * a)), 0.6 * (g_U / 170.0)));
            p->drawLine(QPointF(x - r * 3, y), QPointF(x + r * 3, y));
            p->drawLine(QPointF(x, y - r * 3), QPointF(x, y + r * 3));
            p->setPen(Qt::NoPen);
        }
    }
}

static void draw_crescent(QPainter* p, double cx, double cy, double r, const QColor& color, double thickness = 0.78, double tilt = -20.0) {
    QPainterPath path;
    path.addEllipse(QPointF(cx, cy), r, r);
    QPainterPath cut;
    double off = r * (1.0 - thickness) * 1.9;
    cut.addEllipse(QPointF(cx + off, cy - off * 0.35), r * 0.9, r * 0.9);
    p->save();
    p->translate(cx, cy);
    p->rotate(tilt);
    p->translate(-cx, -cy);
    p->setPen(Qt::NoPen);
    p->setBrush(color);
    p->drawPath(path.subtracted(cut));
    p->restore();
}

struct CloudSpec {
    double x, y, rx, ry, a;
};

static void draw_clouds(QPainter* p, double w, double h, const QColor& color, const QVector<CloudSpec>& items) {
    p->setPen(Qt::NoPen);
    const std::tuple<double, double, double> blobs[] = {
        {0.0, 0.0, 1.0}, {-0.55, 0.1, 0.7}, {0.6, 0.12, 0.75}, {0.15, -0.2, 0.65}
    };
    for (const auto& it : items) {
        for (const auto& [dx, dy, k] : blobs) {
            QColor col0(color); col0.setAlphaF(static_cast<float>(it.a));
            QColor col1(color); col1.setAlphaF(0.0f);
            double cx = it.x * w + dx * it.rx * g_U;
            double cy = (it.y + dy * it.ry) * h;
            auto g = make_rg(cx, cy, it.rx * g_U * k, {{0.0, col0}, {1.0, col1}});
            p->setBrush(QBrush(g));
            p->save();
            p->translate(cx, cy);
            p->scale(1.0, (it.ry / it.rx) * (g_U / h));
            p->translate(-cx, -cy);
            p->drawEllipse(QPointF(cx, cy), it.rx * g_U * k, it.rx * g_U * k);
            p->restore();
        }
    }
}

static void draw_rays(QPainter* p, double cx, double cy, double length, const QColor& color, int n = 14, double spread = PI, double start = PI, double alpha = 0.10) {
    p->setPen(Qt::NoPen);
    for (int i = 0; i < n; ++i) {
        double a = start + spread * (i + 0.5) / n;
        double wdt = (spread / n) * 0.42;
        QPainterPath path(QPointF(cx, cy));
        path.lineTo(cx + length * std::cos(a - wdt), cy + length * std::sin(a - wdt));
        path.lineTo(cx + length * std::cos(a + wdt), cy + length * std::sin(a + wdt));
        path.closeSubpath();
        QLinearGradient g(cx, cy, cx + length * std::cos(a), cy + length * std::sin(a));
        QColor c0(color); c0.setAlphaF(static_cast<float>(alpha));
        QColor c1(color); c1.setAlphaF(0.0f);
        g.setColorAt(0, c0);
        g.setColorAt(1, c1);
        p->setBrush(QBrush(g));
        p->drawPath(path);
    }
}

static QPainterPath make_onion(double cx, double base_y, double r, double height, double pointy = 0.35) {
    QPainterPath path(QPointF(cx - r, base_y));
    path.cubicTo(cx - r * 1.14, base_y - height * 0.50, cx - r * pointy, base_y - height * 0.78, cx, base_y - height);
    path.cubicTo(cx + r * pointy, base_y - height * 0.78, cx + r * 1.14, base_y - height * 0.50, cx + r, base_y);
    path.closeSubpath();
    return path;
}

static QPainterPath make_arch(double cx, double base_y, double w, double h) {
    QPainterPath path(QPointF(cx - w / 2.0, base_y));
    path.lineTo(cx - w / 2.0, base_y - h * 0.55);
    path.cubicTo(cx - w / 2.0, base_y - h * 0.85, cx - w * 0.12, base_y - h * 0.92, cx, base_y - h);
    path.cubicTo(cx + w * 0.12, base_y - h * 0.92, cx + w / 2.0, base_y - h * 0.85, cx + w / 2.0, base_y - h * 0.55);
    path.lineTo(cx + w / 2.0, base_y);
    path.closeSubpath();
    return path;
}

static QBrush make_shaded(const QRectF& rect, const QColor& light, const QColor& dark, bool horizontal = true) {
    QLinearGradient g = horizontal
                            ? QLinearGradient(rect.left(), 0, rect.right(), 0)
                            : QLinearGradient(0, rect.top(), 0, rect.bottom());
    g.setColorAt(0.0, light);
    g.setColorAt(0.55, light);
    g.setColorAt(1.0, dark);
    return QBrush(g);
}

static void draw_finial(QPainter* p, double x, double y, double u, const QColor& color, std::optional<QColor> gold = std::nullopt) {
    QColor col = gold.value_or(color);
    p->setPen(QPen(col, std::max(1.0, u * 0.006)));
    p->drawLine(QPointF(x, y), QPointF(x, y - u * 0.045));
    p->setPen(Qt::NoPen);
    p->setBrush(col);
    p->drawEllipse(QPointF(x, y - u * 0.05), u * 0.011, u * 0.011);
}

static void draw_minaret(QPainter* p, double x, double by, double u, const Scene& sc) {
    QColor body(sc.body), shade(sc.shade), dome(sc.dome);
    double sw = u * 0.040;
    double top1 = by - u * 0.40;
    QRectF r1(x - sw / 2.0, top1, sw, by - top1);
    p->setPen(Qt::NoPen);
    p->setBrush(sc.sil ? QBrush(body) : make_shaded(r1, body, shade));
    p->drawRect(r1);
    double bw = u * 0.082;
    p->setBrush(sc.sil ? body : shade.lighter(112));
    p->drawRect(QRectF(x - bw / 2.0, top1 - u * 0.018, bw, u * 0.018));
    p->drawRect(QRectF(x - bw * 0.42, top1 - u * 0.034, bw * 0.84, u * 0.016));
    double top2 = top1 - u * 0.13;
    QRectF r2(x - sw * 0.38, top2, sw * 0.76, top1 - u * 0.034 - top2);
    p->setBrush(sc.sil ? QBrush(body) : make_shaded(r2, body, shade));
    p->drawRect(r2);
    p->drawRect(QRectF(x - sw * 0.62, top2 - u * 0.008, sw * 1.24, u * 0.012));
    QPainterPath cap = make_onion(x, top2 - u * 0.008, sw * 0.62, u * 0.105, 0.2);
    p->setBrush(dome);
    p->drawPath(cap);
    draw_finial(p, x, top2 - u * 0.113, u, sc.sil ? dome : QColor(QStringLiteral("#D6A84A")));
    if (!sc.windows.isEmpty()) {
        p->setBrush(QColor(sc.windows));
        for (int k = 0; k < 3; ++k) {
            p->drawRoundedRect(QRectF(x - u * 0.004, top1 + u * 0.05 + k * u * 0.095, u * 0.008, u * 0.036), u * 0.004, u * 0.004);
        }
    }
}

static void draw_mosque(QPainter* p, double cx, double by, double u, const Scene& sc) {
    QColor body(sc.body), shade(sc.shade), dome(sc.dome), dshade(sc.dome_shade);
    p->setPen(Qt::NoPen);
    for (int sx : {-1, 1}) {
        draw_minaret(p, cx + sx * u * 0.395, by, u, sc);
    }
    QRectF hall(cx - u * 0.31, by - u * 0.25, u * 0.62, u * 0.25);
    p->setBrush(sc.sil ? QBrush(body) : make_shaded(hall, body, shade));
    p->drawRect(hall);
    p->setBrush(sc.sil ? body : shade);
    p->drawRect(QRectF(hall.left() - u * 0.01, hall.top() - u * 0.012, hall.width() + u * 0.02, u * 0.016));

    // side domes
    for (int sx : {-1, 1}) {
        double x = cx + sx * u * 0.215;
        QPainterPath path = make_onion(x, hall.top() - u * 0.004, u * 0.088, u * 0.13, 0.3);
        p->setBrush(sc.sil ? QBrush(dome) : make_shaded(path.boundingRect(), dome, dshade));
        p->drawPath(path);
        draw_finial(p, x, hall.top() - u * 0.134, u, sc.sil ? dome : QColor(QStringLiteral("#D6A84A")));
    }
    // drum
    QRectF drum(cx - u * 0.155, hall.top() - u * 0.075, u * 0.31, u * 0.075);
    p->setBrush(sc.sil ? QBrush(body) : make_shaded(drum, body, shade));
    p->drawRect(drum);
    // central dome
    QPainterPath cpath = make_onion(cx, drum.top() + u * 0.002, u * 0.175, u * 0.285, 0.30);
    p->setBrush(sc.sil ? QBrush(dome) : make_shaded(cpath.boundingRect(), dome, dshade));
    p->drawPath(cpath);
    if (!sc.sil) {
        QColor hl(QStringLiteral("#FFFFFF"));
        hl.setAlphaF(0.22f);
        p->setPen(QPen(hl, u * 0.006));
        p->setBrush(Qt::NoBrush);
        p->drawArc(QRectF(cx - u * 0.12, drum.top() - u * 0.22, u * 0.14, u * 0.2), 100 * 16, 60 * 16);
        p->setPen(Qt::NoPen);
    }
    draw_finial(p, cx, drum.top() - u * 0.285, u, sc.sil ? dome : QColor(QStringLiteral("#D6A84A")));

    // openings
    QColor win = !sc.windows.isEmpty() ? QColor(sc.windows) : (!sc.sil ? shade.darker(150) : shade);
    p->setBrush(win);
    p->drawPath(make_arch(cx, by, u * 0.13, u * 0.19));
    for (int sx : {-1, 1}) {
        for (int k : {1, 2}) {
            p->drawPath(make_arch(cx + sx * (u * 0.12 + k * u * 0.075), by - u * 0.03, u * 0.036, u * 0.115));
        }
    }
    for (int k = -2; k <= 2; ++k) {
        p->drawRoundedRect(QRectF(cx + k * u * 0.058 - u * 0.011, drum.top() + u * 0.02, u * 0.022, u * 0.038), u * 0.011, u * 0.011);
    }
    if (!sc.windows.isEmpty()) {
        auto g = make_rg(cx, by - u * 0.08, u * 0.12, {{0.0, QColor(255, 220, 140, 150)}, {1.0, QColor(255, 200, 100, 0)}});
        p->setBrush(QBrush(g));
        p->drawEllipse(QPointF(cx, by - u * 0.08), u * 0.12, u * 0.12);
    }
}

static void draw_far_skyline(QPainter* p, double w, double by, double u, const QColor& color, unsigned int seed) {
    std::mt19937 rnd(seed);
    auto uniform = [&](double a, double b) {
        return std::uniform_real_distribution<double>(a, b)(rnd);
    };
    p->setPen(Qt::NoPen);
    p->setBrush(color);
    double x = -u * 0.05;
    while (x < w + u * 0.05) {
        double bw = u * uniform(0.10, 0.20);
        double bh = u * uniform(0.05, 0.12);
        p->drawRect(QRectF(x, by - bh, bw, bh + u));
        double r = uniform(0.0, 1.0);
        if (r < 0.42) {
            p->drawPath(make_onion(x + bw / 2.0, by - bh, bw * 0.38, bw * 0.55, 0.3));
        } else if (r < 0.68) {
            double mw = u * 0.022;
            p->drawRect(QRectF(x + bw * 0.5 - mw / 2.0, by - bh - u * uniform(0.12, 0.22), mw, u * 0.3));
            p->drawPath(make_onion(x + bw * 0.5, by - bh - u * 0.18, mw * 0.8, mw * 2.4, 0.2));
        }
        x += bw * uniform(0.75, 1.0);
    }
}

static void draw_palm(QPainter* p, double x, double by, double h, const QColor& color, double lean = 0.1) {
    p->setPen(Qt::NoPen);
    p->setBrush(color);
    double tw = h * 0.035;
    double tx = x + lean * h;
    double ty = by - h;
    QPainterPath trunk(QPointF(x - tw, by));
    trunk.quadTo(x + lean * h * 0.2, by - h * 0.5, tx - tw * 0.4, ty);
    trunk.lineTo(tx + tw * 0.4, ty);
    trunk.quadTo(x + lean * h * 0.2 + tw, by - h * 0.5, x + tw, by);
    trunk.closeSubpath();
    p->drawPath(trunk);

    const double angles[] = {-160, -135, -108, -80, -52, -25, 0, 20, -190};
    for (int k = 0; k < 9; ++k) {
        double a = angles[k] * PI / 180.0;
        double L = h * ((k % 2) ? 0.42 : 0.50);
        double ex = tx + L * std::cos(a);
        double ey = ty + L * std::sin(a) + L * 0.42;
        double mx = tx + L * 0.55 * std::cos(a);
        double my = ty + L * 0.55 * std::sin(a) - L * 0.12;
        QPainterPath leaf(QPointF(tx, ty));
        leaf.quadTo(mx, my - h * 0.025, ex, ey);
        leaf.quadTo(mx, my + h * 0.03, tx, ty);
        p->drawPath(leaf);
    }
}

struct BirdPt {
    double x, y, k;
};

static void draw_birds(QPainter* p, const QVector<BirdPt>& pts, const QColor& color, double size) {
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(color, std::max(1.0, size * 0.18), Qt::SolidLine, Qt::RoundCap));
    for (const auto& pt : pts) {
        double s = size * pt.k;
        QPainterPath path(QPointF(pt.x - s, pt.y - s * 0.2));
        path.quadTo(pt.x - s * 0.5, pt.y - s * 0.7, pt.x, pt.y);
        path.quadTo(pt.x + s * 0.5, pt.y - s * 0.7, pt.x + s, pt.y - s * 0.2);
        p->drawPath(path);
    }
}

QPixmap render(const QString& key, int w, int h, double dpr, std::optional<double> unit, std::optional<double> cx) {
    const auto& sc = scenes().value(key, scenes().value(QStringLiteral("fajr")));
    QPixmap pm(static_cast<int>(w * dpr), static_cast<int>(h * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    double u = unit.value_or(static_cast<double>(w));
    g_U = u;
    double mx = cx.value_or(w / 2.0);
    double by = h * 0.775;

    // sky
    QLinearGradient g(0, 0, 0, by + u * 0.05);
    for (const auto& [pos, col] : sc.sky) {
        g.setColorAt(pos, QColor(col));
    }
    p.fillRect(QRectF(0, 0, w, h), QBrush(g));
    bool night = (key == QStringLiteral("fajr") || key == QStringLiteral("isha"));

    // celestial + atmosphere per prayer
    if (key == QStringLiteral("fajr")) {
        draw_stars(&p, w, h, 70, 11, 0.55, 0.9);
        draw_glow(&p, w * 0.5, by, u * 0.9, QColor(QStringLiteral("#F4A98F")), 0.55);
        draw_glow(&p, w * 0.70, h * 0.17, u * 0.34, QColor(QStringLiteral("#CFC8FF")), 0.45);
        draw_crescent(&p, w * 0.70, h * 0.17, u * 0.075, QColor(QStringLiteral("#F7F0D6")), 0.72, -28);
        draw_clouds(&p, w, h, QColor(QStringLiteral("#D890B0")), {{0.25, 0.46, 0.33, 0.035, 0.30}, {0.78, 0.55, 0.30, 0.03, 0.26}});
    } else if (key == QStringLiteral("sunrise")) {
        double sx = w * 0.75, sy = by - u * 0.31;
        draw_glow(&p, sx, sy, u * 1.15, QColor(QStringLiteral("#FFD27A")), 0.95);
        draw_rays(&p, sx, sy, h * 0.9, QColor(QStringLiteral("#FFE9B0")), 16, PI, PI, 0.17);
        auto gg = make_rg(sx, sy, u * 0.17, {{0.0, QColor(QStringLiteral("#FFF8DC"))}, {0.7, QColor(QStringLiteral("#FFE19A"))}, {1.0, QColor(QStringLiteral("#FFC768"))}});
        p.setPen(Qt::NoPen); p.setBrush(QBrush(gg)); p.drawEllipse(QPointF(sx, sy), u * 0.17, u * 0.17);
        draw_stars(&p, w, h, 14, 5, 0.25, 0.5);
        draw_clouds(&p, w, h, QColor(QStringLiteral("#FFD7A8")), {{0.22, 0.40, 0.34, 0.03, 0.40}, {0.80, 0.30, 0.30, 0.028, 0.34}, {0.55, 0.52, 0.36, 0.03, 0.30}});
    } else if (key == QStringLiteral("dhuhr")) {
        double sx = w * 0.66, sy = h * 0.15;
        draw_glow(&p, sx, sy, u * 0.95, QColor(QStringLiteral("#FFFFFF")), 0.85);
        draw_rays(&p, sx, sy, h * 1.1, QColor(QStringLiteral("#FFFFFF")), 18, PI * 0.95, PI * 0.02, 0.10);
        p.setPen(Qt::NoPen); p.setBrush(QColor(QStringLiteral("#FFFDF0"))); p.drawEllipse(QPointF(sx, sy), u * 0.095, u * 0.095);
        draw_clouds(&p, w, h, QColor(QStringLiteral("#FFFFFF")), {{0.22, 0.30, 0.30, 0.03, 0.85}, {0.80, 0.44, 0.34, 0.034, 0.80}, {0.38, 0.55, 0.32, 0.03, 0.70}});
    } else if (key == QStringLiteral("asr")) {
        double sx = w * 0.80, sy = h * 0.36;
        draw_glow(&p, sx, sy, u * 1.1, QColor(QStringLiteral("#FFD58A")), 0.9);
        draw_rays(&p, sx, sy, h * 0.9, QColor(QStringLiteral("#FFE2A8")), 14, PI * 0.9, PI * 0.55, 0.12);
        p.setPen(Qt::NoPen); p.setBrush(QColor(QStringLiteral("#FFF0C0"))); p.drawEllipse(QPointF(sx, sy), u * 0.10, u * 0.10);
        draw_clouds(&p, w, h, QColor(QStringLiteral("#FBE3B6")), {{0.28, 0.32, 0.34, 0.03, 0.55}, {0.62, 0.48, 0.36, 0.03, 0.45}});
    } else if (key == QStringLiteral("maghrib")) {
        double sx = w * 0.26, sy = by - u * 0.30;
        draw_glow(&p, sx, sy, u * 1.25, QColor(QStringLiteral("#FF8A4B")), 0.95);
        p.setPen(Qt::NoPen);
        auto gg = make_rg(sx, sy, u * 0.21, {{0.0, QColor(QStringLiteral("#FFE3A3"))}, {0.6, QColor(QStringLiteral("#FF9A55"))}, {1.0, QColor(QStringLiteral("#F0603F"))}});
        p.setBrush(QBrush(gg)); p.drawEllipse(QPointF(sx, sy), u * 0.21, u * 0.21);
        draw_clouds(&p, w, h, QColor(QStringLiteral("#FF9A7A")), {{0.30, 0.36, 0.36, 0.03, 0.50}, {0.78, 0.30, 0.30, 0.026, 0.42}, {0.6, 0.50, 0.36, 0.03, 0.40}});
        draw_stars(&p, w, h, 18, 3, 0.22, 0.6);
    } else if (key == QStringLiteral("isha")) {
        draw_stars(&p, w, h, 120, 29, 0.66, 1.0);
        draw_glow(&p, w * 0.30, h * 0.20, u * 0.60, QColor(QStringLiteral("#9DB4FF")), 0.40);
        draw_glow(&p, w * 0.30, h * 0.20, u * 0.20, QColor(QStringLiteral("#FFF4D0")), 0.55);
        draw_crescent(&p, w * 0.30, h * 0.20, u * 0.115, QColor(QStringLiteral("#FFF6DA")), 0.74, -26);
        draw_glow(&p, w * 0.5, by, u * 0.9, QColor(QStringLiteral("#3A6AA8")), 0.35);
    }

    // far skyline
    unsigned int kseed = static_cast<unsigned int>(qHash(key) % 997);
    draw_far_skyline(&p, w, by + u * 0.01, u, QColor(sc.far_col), kseed);

    // mosque
    draw_mosque(&p, mx, by, u, sc);

    // ground
    QLinearGradient gr(0, by, 0, h);
    gr.setColorAt(0, QColor(sc.ground.first));
    gr.setColorAt(1, QColor(sc.ground.second));
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(gr));
    p.drawRect(QRectF(0, by, w, h - by + 1));

    if (key == QStringLiteral("asr")) {
        QPainterPath sh(QPointF(w * 0.18, by));
        sh.lineTo(w * 0.82, by);
        sh.lineTo(w * 0.40, by + (h - by) * 0.72);
        sh.lineTo(-w * 0.15, by + (h - by) * 0.72);
        sh.closeSubpath();
        p.setBrush(QColor(60, 36, 12, 70));
        p.drawPath(sh);
    }
    if (key == QStringLiteral("dhuhr")) {
        p.setBrush(QColor(255, 255, 255, 60));
        p.drawRect(QRectF(0, by, w, u * 0.012));
    }

    // reflection of the mosque on the courtyard
    QImage img = pm.toImage();
    int sh = static_cast<int>(std::min((h - by) * dpr * 0.9, by * dpr));
    if (sh > 1) {
        #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        QImage src = img.copy(0, static_cast<int>(by * dpr) - sh, static_cast<int>(w * dpr), sh).flipped(Qt::Vertical);
#else
        QImage src = img.copy(0, static_cast<int>(by * dpr) - sh, static_cast<int>(w * dpr), sh).mirrored(false, true);
#endif
        QPainter mask(&src);
        mask.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        QLinearGradient mg(0, 0, 0, sh);
        mg.setColorAt(0, QColor(0, 0, 0, night ? 90 : 60));
        mg.setColorAt(1, QColor(0, 0, 0, 0));
        mask.fillRect(src.rect(), QBrush(mg));
        mask.end();
        p.drawImage(QRectF(0, by, w, sh / dpr), src);
    }

    // palms / birds
    QColor pal(sc.ground.second);
    if (key == QStringLiteral("fajr") || key == QStringLiteral("maghrib") ||
        key == QStringLiteral("isha") || key == QStringLiteral("sunrise")) {
        draw_palm(&p, -w * 0.01, by + u * 0.015, u * 0.55, pal, 0.13);
        draw_palm(&p, w * 1.01, by + u * 0.015, u * 0.46, pal, -0.12);
    }
    if (key == QStringLiteral("asr")) {
        draw_palm(&p, w * 1.01, by + u * 0.02, u * 0.50, QColor(QStringLiteral("#6E5128")), -0.12);
    }
    if (key == QStringLiteral("maghrib")) {
        draw_birds(&p, {{w * 0.62, h * 0.24, 1.0}, {w * 0.70, h * 0.20, 0.8}, {w * 0.56, h * 0.19, 0.7}, {w * 0.76, h * 0.27, 0.6}, {w * 0.66, h * 0.30, 0.5}},
                   QColor(QStringLiteral("#2A1228")), u * 0.035);
    }
    if (key == QStringLiteral("dhuhr")) {
        draw_birds(&p, {{w * 0.30, h * 0.20, 1.0}, {w * 0.37, h * 0.17, 0.7}},
                   QColor(QStringLiteral("#4B6A80")), u * 0.03);
    }

    // horizon haze + vignette
    QLinearGradient hz(0, by - u * 0.22, 0, by + u * 0.02);
    QColor hc(sc.sky.last().second);
    QColor hc0(hc); hc0.setAlphaF(0.0f);
    QColor hc1(hc); hc1.setAlphaF(night ? 0.10f : 0.30f);
    hz.setColorAt(0, hc0);
    hz.setColorAt(1, hc1);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(hz));
    p.drawRect(QRectF(0, by - u * 0.22, w, u * 0.24));

    auto vg = make_rg(w / 2.0, h * 0.45, h * 0.85, {{0.0, QColor(0, 0, 0, 0)}, {0.65, QColor(0, 0, 0, 0)}, {1.0, QColor(0, 0, 0, night ? 70 : 38)}});
    p.setBrush(QBrush(vg));
    p.drawRect(QRectF(0, 0, w, h));
    p.end();
    return pm;
}

} // namespace vm::ui::art
