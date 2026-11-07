#include "icons.h"
#include "../theme.h"
#include <QBrush>
#include <QLinearGradient>
#include <QMap>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QRadialGradient>
#include <QTransform>
#include <QVector>
#include <cmath>
#include <functional>

namespace vm::ui::icons {

static constexpr double PI = 3.14159265358979323846;

static QPen make_pen(const QColor& c, double w) {
    QPen p(c, w);
    p.setCapStyle(Qt::RoundCap);
    p.setJoinStyle(Qt::RoundJoin);
    return p;
}

static QPainterPath make_poly(const QVector<QPointF>& pts) {
    QPainterPath path;
    if (pts.isEmpty()) return path;
    path.moveTo(pts.first());
    for (int i = 1; i < pts.size(); ++i) {
        path.lineTo(pts[i]);
    }
    return path;
}

static void ic_home(QPainter* p, const QColor&) {
    QPainterPath path;
    path.moveTo(3.5, 11); path.lineTo(12, 3.8); path.lineTo(20.5, 11);
    p->drawPath(path);
    QPainterPath b;
    b.moveTo(5.5, 9.6); b.lineTo(5.5, 19.5); b.quadTo(5.5, 20.5, 6.5, 20.5); b.lineTo(17.5, 20.5);
    b.quadTo(18.5, 20.5, 18.5, 19.5); b.lineTo(18.5, 9.6);
    p->drawPath(b);
    p->drawPath(make_poly({{10, 20.5}, {10, 15}, {14, 15}, {14, 20.5}}));
}

static void ic_gear(QPainter* p, const QColor&) {
    double cx = 12, cy = 12;
    QPainterPath path;
    int n = 8;
    for (int i = 0; i < n * 2; ++i) {
        double ang = PI * i / n;
        double rr = (i % 2 == 0) ? 9.2 : 7.0;
        for (double off : {-0.16, 0.16}) {
            double a = ang + off;
            QPointF pt(cx + rr * std::cos(a), cy + rr * std::sin(a));
            if (path.elementCount() == 0) path.moveTo(pt);
            else path.lineTo(pt);
        }
    }
    path.closeSubpath();
    p->drawPath(path);
    p->drawEllipse(QPointF(12, 12), 3.2, 3.2);
}

static void ic_moon(QPainter* p, const QColor&) {
    QPainterPath path;
    path.addEllipse(QPointF(12, 12), 8.5, 8.5);
    QPainterPath cut;
    cut.addEllipse(QPointF(16.2, 8.6), 7.2, 7.2);
    p->drawPath(path.subtracted(cut));
}

static void ic_sun(QPainter* p, const QColor&) {
    p->drawEllipse(QPointF(12, 12), 4.2, 4.2);
    for (int i = 0; i < 8; ++i) {
        double a = (PI / 4.0) * i;
        p->drawLine(QPointF(12 + 7.4 * std::cos(a), 12 + 7.4 * std::sin(a)),
                    QPointF(12 + 9.6 * std::cos(a), 12 + 9.6 * std::sin(a)));
    }
}

static void ic_bell(QPainter* p, const QColor&) {
    QPainterPath path;
    path.moveTo(5.5, 17); path.quadTo(7.5, 15, 7.5, 10.5);
    path.cubicTo(7.5, 5.5, 16.5, 5.5, 16.5, 10.5);
    path.quadTo(16.5, 15, 18.5, 17); path.closeSubpath();
    p->drawPath(path);
    p->drawArc(QRectF(9.6, 17.6, 4.8, 3.6), 180 * 16, 180 * 16);
    p->drawLine(QPointF(12, 4.2), QPointF(12, 5.6));
}

static void ic_bell_off(QPainter* p, const QColor& c) {
    ic_bell(p, c);
    p->drawLine(QPointF(4, 4), QPointF(20, 20));
}

static void ic_pin(QPainter* p, const QColor&) {
    QPainterPath path;
    path.moveTo(12, 21);
    path.cubicTo(5.5, 14.5, 4.8, 12, 4.8, 9.6);
    path.cubicTo(4.8, 5.6, 8, 3, 12, 3);
    path.cubicTo(16, 3, 19.2, 5.6, 19.2, 9.6);
    path.cubicTo(19.2, 12, 18.5, 14.5, 12, 21);
    p->drawPath(path);
    p->drawEllipse(QPointF(12, 9.6), 2.6, 2.6);
}

static void ic_globe(QPainter* p, const QColor&) {
    p->drawEllipse(QPointF(12, 12), 8.8, 8.8);
    p->drawEllipse(QPointF(12, 12), 3.8, 8.8);
    p->drawLine(QPointF(3.2, 12), QPointF(20.8, 12));
    p->drawArc(QRectF(4.8, 5.2, 14.4, 7), 0, 180 * 16);
    p->drawArc(QRectF(4.8, 11.8, 14.4, 7), 180 * 16, 180 * 16);
}

static void ic_check(QPainter* p, const QColor&) {
    p->drawPath(make_poly({{5, 12.5}, {10, 17.5}, {19, 7}}));
}

static void ic_x(QPainter* p, const QColor&) {
    p->drawLine(QPointF(6, 6), QPointF(18, 18));
    p->drawLine(QPointF(18, 6), QPointF(6, 18));
}

static void ic_minus(QPainter* p, const QColor&) {
    p->drawLine(QPointF(6, 12), QPointF(18, 12));
}

static void ic_plus(QPainter* p, const QColor&) {
    p->drawLine(QPointF(6, 12), QPointF(18, 12));
    p->drawLine(QPointF(12, 6), QPointF(12, 18));
}

static void ic_square(QPainter* p, const QColor&) {
    p->drawRoundedRect(QRectF(6, 6, 12, 12), 2, 2);
}

static void ic_restore(QPainter* p, const QColor&) {
    p->drawRoundedRect(QRectF(5, 8.5, 10.5, 10.5), 2, 2);
    p->drawPath(make_poly({{9, 8.5}, {9, 5.5}, {19, 5.5}, {19, 15.5}, {15.5, 15.5}}));
}

static void ic_search(QPainter* p, const QColor&) {
    p->drawEllipse(QPointF(10.5, 10.5), 6.2, 6.2);
    p->drawLine(QPointF(15.2, 15.2), QPointF(20, 20));
}

static void ic_chev_down(QPainter* p, const QColor&) {
    p->drawPath(make_poly({{6.5, 9.5}, {12, 15}, {17.5, 9.5}}));
}

static void ic_chev_right(QPainter* p, const QColor&) {
    p->drawPath(make_poly({{9.5, 6.5}, {15, 12}, {9.5, 17.5}}));
}

static void ic_chev_left(QPainter* p, const QColor&) {
    p->drawPath(make_poly({{14.5, 6.5}, {9, 12}, {14.5, 17.5}}));
}

static void ic_refresh(QPainter* p, const QColor&) {
    p->drawArc(QRectF(4.5, 4.5, 15, 15), 40 * 16, 270 * 16);
    p->drawPath(make_poly({{17.2, 3.2}, {18.4, 7.6}, {14, 8.4}}));
}

static void ic_play(QPainter* p, const QColor& c) {
    p->setBrush(QBrush(c));
    p->setPen(make_pen(c, 1.2));
    p->drawPath(make_poly({{8, 5.8}, {18.5, 12}, {8, 18.2}}));
}

static void ic_stop(QPainter* p, const QColor& c) {
    p->setBrush(QBrush(c));
    p->setPen(make_pen(c, 1.2));
    p->drawRoundedRect(QRectF(6.5, 6.5, 11, 11), 2, 2);
}

static void ic_volume(QPainter* p, const QColor&) {
    p->drawPath(make_poly({{4.5, 9.5}, {8.2, 9.5}, {12.5, 5.8}, {12.5, 18.2}, {8.2, 14.5}, {4.5, 14.5}}));
    p->drawArc(QRectF(10, 8.2, 7.6, 7.6), -50 * 16, 100 * 16);
    p->drawArc(QRectF(8.2, 5.4, 12.4, 12.8), -52 * 16, 104 * 16);
}

static void ic_clock(QPainter* p, const QColor&) {
    p->drawEllipse(QPointF(12, 12), 8.6, 8.6);
    p->drawPath(make_poly({{12, 7}, {12, 12}, {15.4, 14}}));
}

static void ic_compass(QPainter* p, const QColor& c) {
    p->drawEllipse(QPointF(12, 12), 8.8, 8.8);
    p->setBrush(QBrush(c));
    p->drawPath(make_poly({{15.8, 8.2}, {13.4, 13.4}, {8.2, 15.8}, {10.6, 10.6}}));
}

static void ic_snooze(QPainter* p, const QColor&) {
    p->drawEllipse(QPointF(12, 13), 7.4, 7.4);
    p->drawPath(make_poly({{9.2, 11}, {14.6, 11}, {9.2, 15.2}, {14.8, 15.2}}));
    p->drawLine(QPointF(4.2, 6), QPointF(7.2, 3.6));
    p->drawLine(QPointF(19.8, 6), QPointF(16.8, 3.6));
}

static void ic_mosque(QPainter* p, const QColor&) {
    p->drawArc(QRectF(6.5, 7, 11, 11), 0, 180 * 16);
    p->drawLine(QPointF(6.5, 12.5), QPointF(6.5, 20.5));
    p->drawLine(QPointF(17.5, 12.5), QPointF(17.5, 20.5));
    p->drawLine(QPointF(4, 20.5), QPointF(20, 20.5));
    p->drawLine(QPointF(12, 7), QPointF(12, 4.2));
    p->drawLine(QPointF(10.6, 20.5), QPointF(10.6, 16.4));
    p->drawLine(QPointF(13.4, 20.5), QPointF(13.4, 16.4));
    p->drawArc(QRectF(10.6, 14.6, 2.8, 3.6), 0, 180 * 16);
}

static void ic_folder(QPainter* p, const QColor&) {
    p->drawPath(make_poly({{3.8, 7.5}, {3.8, 18.5}, {20.2, 18.5}, {20.2, 9}, {11, 9}, {9.4, 6.5}, {3.8, 6.5}, {3.8, 7.5}}));
}

static void ic_locate(QPainter* p, const QColor& c) {
    p->drawEllipse(QPointF(12, 12), 5.2, 5.2);
    p->setBrush(QBrush(c));
    p->drawEllipse(QPointF(12, 12), 1.8, 1.8);
    p->setBrush(Qt::NoBrush);
    p->drawLine(QPointF(12, 2.8), QPointF(12, 5.6));
    p->drawLine(QPointF(12, 18.4), QPointF(12, 21.2));
    p->drawLine(QPointF(2.8, 12), QPointF(5.6, 12));
    p->drawLine(QPointF(18.4, 12), QPointF(21.2, 12));
}

static void ic_database(QPainter* p, const QColor&) {
    p->drawEllipse(QRectF(5.5, 4.5, 13, 5));
    p->drawArc(QRectF(5.5, 9.5, 13, 5), 180 * 16, 180 * 16);
    p->drawArc(QRectF(5.5, 14.5, 13, 5), 180 * 16, 180 * 16);
    p->drawLine(QPointF(5.5, 7), QPointF(5.5, 17));
    p->drawLine(QPointF(18.5, 7), QPointF(18.5, 17));
}

static void ic_palette(QPainter* p, const QColor& c) {
    QPainterPath path;
    path.moveTo(12, 3.2);
    path.cubicTo(6.4, 3.2, 3, 7, 3, 11.6);
    path.cubicTo(3, 16.4, 6.8, 20.4, 12, 20.4);
    path.cubicTo(13.6, 20.4, 14.2, 19.4, 13.6, 18.2);
    path.cubicTo(12.8, 16.6, 14, 15.4, 15.6, 15.4);
    path.lineTo(17.4, 15.4);
    path.cubicTo(19.6, 15.4, 21, 13.8, 21, 11.8);
    path.cubicTo(21, 6.8, 17, 3.2, 12, 3.2);
    p->drawPath(path);
    p->setBrush(QBrush(c));
    p->setPen(Qt::NoPen);
    for (const auto& pt : {QPointF(7.8, 11), QPointF(10.2, 7.4), QPointF(14.6, 7.8)}) {
        p->drawEllipse(pt, 1.35, 1.35);
    }
}

static void ic_info(QPainter* p, const QColor& c) {
    p->drawEllipse(QPointF(12, 12), 8.8, 8.8);
    p->drawLine(QPointF(12, 11), QPointF(12, 16.4));
    p->setBrush(QBrush(c));
    p->drawEllipse(QPointF(12, 7.8), 1.1, 1.1);
    p->setBrush(Qt::NoBrush);
}

static void ic_mail(QPainter* p, const QColor&) {
    p->drawRoundedRect(QRectF(3.4, 5.6, 17.2, 12.8), 2.6, 2.6);
    p->drawPath(make_poly({{4.2, 7.2}, {12, 13}, {19.8, 7.2}}));
}

static void ic_phone(QPainter* p, const QColor&) {
    p->drawRoundedRect(QRectF(7, 3, 10, 18), 2.6, 2.6);
    p->drawLine(QPointF(10.6, 17.8), QPointF(13.4, 17.8));
}

static void ic_copy(QPainter* p, const QColor&) {
    p->drawRoundedRect(QRectF(8.6, 8.6, 11, 11), 2.4, 2.4);
    p->drawPath(make_poly({{15.4, 8.6}, {15.4, 5.8}, {14.2, 4.6}, {6.4, 4.6}, {4.6, 6.4}, {4.6, 14.2}, {5.8, 15.4}, {8.6, 15.4}}));
}

static void ic_heart(QPainter* p, const QColor&) {
    QPainterPath path(QPointF(12, 20));
    path.cubicTo(3.5, 14.2, 3.2, 9.6, 6.2, 7.2);
    path.cubicTo(8.4, 5.5, 11, 6.5, 12, 8.6);
    path.cubicTo(13, 6.5, 15.6, 5.5, 17.8, 7.2);
    path.cubicTo(20.8, 9.6, 20.5, 14.2, 12, 20);
    p->drawPath(path);
}

static void ic_calendar(QPainter* p, const QColor& c) {
    p->drawRoundedRect(QRectF(4, 5.6, 16, 14.6), 3, 3);
    p->drawLine(QPointF(4, 10.2), QPointF(20, 10.2));
    p->drawLine(QPointF(8.4, 3.6), QPointF(8.4, 7.4));
    p->drawLine(QPointF(15.6, 3.6), QPointF(15.6, 7.4));
    p->setBrush(QBrush(c));
    p->drawEllipse(QPointF(9, 14.4), 1, 1);
    p->drawEllipse(QPointF(15, 14.4), 1, 1);
    p->setBrush(Qt::NoBrush);
}

static void ic_power(QPainter* p, const QColor&) {
    p->drawArc(QRectF(5, 5.4, 14, 14), 120 * 16, -300 * 16);
    p->drawLine(QPointF(12, 3.4), QPointF(12, 11.4));
}

using IconFn = void (*)(QPainter*, const QColor&);

static const QMap<QString, IconFn>& icon_map() {
    static const QMap<QString, IconFn> m = {
        {QStringLiteral("calendar"), &ic_calendar},
        {QStringLiteral("power"), &ic_power},
        {QStringLiteral("info"), &ic_info},
        {QStringLiteral("mail"), &ic_mail},
        {QStringLiteral("phone"), &ic_phone},
        {QStringLiteral("copy"), &ic_copy},
        {QStringLiteral("heart"), &ic_heart},
        {QStringLiteral("palette"), &ic_palette},
        {QStringLiteral("home"), &ic_home},
        {QStringLiteral("gear"), &ic_gear},
        {QStringLiteral("moon"), &ic_moon},
        {QStringLiteral("sun"), &ic_sun},
        {QStringLiteral("bell"), &ic_bell},
        {QStringLiteral("bell_off"), &ic_bell_off},
        {QStringLiteral("pin"), &ic_pin},
        {QStringLiteral("globe"), &ic_globe},
        {QStringLiteral("check"), &ic_check},
        {QStringLiteral("x"), &ic_x},
        {QStringLiteral("minus"), &ic_minus},
        {QStringLiteral("plus"), &ic_plus},
        {QStringLiteral("square"), &ic_square},
        {QStringLiteral("restore"), &ic_restore},
        {QStringLiteral("search"), &ic_search},
        {QStringLiteral("chev_down"), &ic_chev_down},
        {QStringLiteral("chev_right"), &ic_chev_right},
        {QStringLiteral("chev_left"), &ic_chev_left},
        {QStringLiteral("refresh"), &ic_refresh},
        {QStringLiteral("play"), &ic_play},
        {QStringLiteral("stop"), &ic_stop},
        {QStringLiteral("volume"), &ic_volume},
        {QStringLiteral("clock"), &ic_clock},
        {QStringLiteral("compass"), &ic_compass},
        {QStringLiteral("snooze"), &ic_snooze},
        {QStringLiteral("mosque"), &ic_mosque},
        {QStringLiteral("folder"), &ic_folder},
        {QStringLiteral("locate"), &ic_locate},
        {QStringLiteral("db"), &ic_database},
    };
    return m;
}

void draw(QPainter* p, const QString& name, const QRectF& r, const QColor& color, std::optional<double> w) {
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    double s = std::min(r.width(), r.height());
    p->translate(r.center().x() - s / 2.0, r.center().y() - s / 2.0);
    p->scale(s / 24.0, s / 24.0);
    p->setPen(make_pen(color, w.value_or(1.9)));
    p->setBrush(Qt::NoBrush);
    auto it = icon_map().find(name);
    if (it != icon_map().end() && it.value()) {
        it.value()(p, color);
    }
    p->restore();
}

QPixmap pixmap(const QString& name, int size, const QColor& color, double dpr, std::optional<double> w) {
    QPixmap pm(static_cast<int>(size * dpr), static_cast<int>(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    draw(&p, name, QRectF(0, 0, size, size), color, w);
    p.end();
    return pm;
}

QIcon icon(const QString& name, int size, const QColor& color) {
    return QIcon(pixmap(name, size, color));
}

static QPainterPath onion_path(double cx, double base_y, double r, double h, double pointy = 0.25) {
    QPainterPath path(QPointF(cx - r, base_y));
    path.cubicTo(cx - r * 1.25, base_y - h * 0.5, cx - r * pointy, base_y - h * 0.78, cx, base_y - h);
    path.cubicTo(cx + r * pointy, base_y - h * 0.78, cx + r * 1.25, base_y - h * 0.5, cx + r, base_y);
    path.closeSubpath();
    return path;
}

static QPainterPath crescent_path(double cx, double cy, double r, double thick = 0.72, double tilt = 0.0) {
    QPainterPath outer;
    outer.addEllipse(QPointF(cx, cy), r, r);
    QPainterPath cut;
    double off = r * (1.0 - thick) * 1.9;
    cut.addEllipse(QPointF(cx + off, cy - off * 0.35), r * 0.9, r * 0.9);
    QPainterPath path = outer.subtracted(cut);
    QTransform tr;
    tr.translate(cx, cy);
    tr.rotate(tilt);
    tr.translate(-cx, -cy);
    return tr.map(path);
}

QPixmap logo_pixmap(int size, double dpr) {
    auto& T = theme::T();
    auto P = T.preview(T.palette, QStringLiteral("dark"));
    QColor acc = P.value(QStringLiteral("accent"));
    QColor hi = acc.lighter(140);
    QColor lo = P.value(QStringLiteral("accent2"));
    QColor deep = lo.darker(135);
    bool detail = size >= 44;

    QPixmap pm(static_cast<int>(size * dpr), static_cast<int>(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.scale(size / 100.0, size / 100.0);

    auto metal = [&](double y0, double y1, QColor a, QColor b, QColor c) -> QBrush {
        QLinearGradient g(0, y0, 0, y1);
        g.setColorAt(0, a);
        g.setColorAt(0.5, b);
        g.setColorAt(1, c);
        return QBrush(g);
    };

    // badge
    QPainterPath badge;
    badge.addRoundedRect(QRectF(0, 0, 100, 100), 27, 27);
    QLinearGradient g(0, 0, 100, 100);
    g.setColorAt(0, P.value(QStringLiteral("bg1")).lighter(142));
    g.setColorAt(1, P.value(QStringLiteral("bg0")).darker(110));
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(g));
    p.drawPath(badge);

    p.save();
    p.setClipPath(badge);
    QRadialGradient glow(34, 14, 78);
    QColor c0 = acc; c0.setAlpha(58);
    QColor c1 = acc; c1.setAlpha(0);
    glow.setColorAt(0, c0);
    glow.setColorAt(1, c1);
    p.setBrush(QBrush(glow));
    p.drawRect(QRectF(0, 0, 100, 100));

    // double eight-pointed star
    auto star = [](double half) -> QPainterPath {
        QPainterPath a;
        a.addRect(QRectF(50 - half, 51 - half, 2 * half, 2 * half));
        QPainterPath b;
        b.addRect(QRectF(50 - half, 51 - half, 2 * half, 2 * half));
        QTransform tr;
        tr.translate(50, 51);
        tr.rotate(45);
        tr.translate(-50, -51);
        return a.united(tr.map(b));
    };
    QColor sc = acc; sc.setAlpha(205);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(sc, 1.5));
    p.drawPath(star(31));
    if (detail) {
        QColor sc2 = acc; sc2.setAlpha(105);
        p.setPen(QPen(sc2, 1.1));
        p.drawPath(star(25.5));
    }
    p.restore();

    // glowing crescent halo
    QRadialGradient halo(50, 41, 17);
    QColor h0 = hi; h0.setAlpha(110);
    QColor h1 = hi; h1.setAlpha(0);
    halo.setColorAt(0, h0);
    halo.setColorAt(1, h1);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(halo));
    p.drawEllipse(QPointF(50, 41), 17, 17);

    // minarets
    for (double cx : {30.75, 69.25}) {
        p.setPen(Qt::NoPen);
        p.setBrush(metal(12, 42, hi, acc, deep));
        p.drawRect(QRectF(cx - 3.3, 27, 6.6, 15));
        p.drawRoundedRect(QRectF(cx - 6.4, 29.6, 12.8, 3.4), 1.2, 1.2);
        p.drawRect(QRectF(cx - 2.5, 21.5, 5.0, 9));
        p.drawPath(onion_path(cx, 22.4, 5.3, 10.6, 0.2));
        p.setPen(QPen(acc, 1.3, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(cx, 11.8), QPointF(cx, 9.6));
        p.setPen(Qt::NoPen);
        if (detail) {
            p.setBrush(QBrush(hi));
            p.drawPath(crescent_path(cx, 7.7, 2.7, 0.7, -20));
            p.setBrush(P.value(QStringLiteral("bg0")));
            for (double k : {-1.6, 0.0, 1.6}) {
                p.drawRect(QRectF(cx + k * 1.7 - 0.45, 30.6, 0.9, 1.4));
            }
        } else {
            p.setBrush(QBrush(hi));
            p.drawEllipse(QPointF(cx, 9.4), 1.7, 1.7);
        }
    }

    // the V
    QPainterPath left(QPointF(24, 40));
    left.lineTo(50, 82); left.lineTo(50, 64); left.lineTo(37.5, 40); left.closeSubpath();
    QPainterPath right(QPointF(76, 40));
    right.lineTo(50, 82); right.lineTo(50, 64); right.lineTo(62.5, 40); right.closeSubpath();

    QLinearGradient gl(24, 40, 50, 82);
    gl.setColorAt(0, hi); gl.setColorAt(0.55, acc); gl.setColorAt(1, lo);
    QLinearGradient gr(76, 40, 50, 82);
    gr.setColorAt(0, acc); gr.setColorAt(0.6, lo); gr.setColorAt(1, deep);

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(gl));
    p.drawPath(left);
    p.setBrush(QBrush(gr));
    p.drawPath(right);

    if (detail) {
        QColor e(255, 255, 255, 105);
        p.setPen(QPen(e, 0.9, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawLine(QPointF(24.4, 40.4), QPointF(50, 81.4));
        QColor s(0, 0, 0, 60);
        p.setPen(QPen(s, 0.8));
        p.drawLine(QPointF(50, 64.5), QPointF(50, 81.5));
    }

    // crescent between minarets
    p.setPen(Qt::NoPen);
    p.setBrush(metal(32, 50, hi, hi.darker(108), acc));
    p.drawPath(crescent_path(50, 41.5, 8.2, 0.72, -28));
    if (detail) {
        p.setBrush(QBrush(hi));
        p.drawEllipse(QPointF(53.6, 41.2), 1.1, 1.1);
    }

    // inner rim
    QColor rim = acc; rim.setAlpha(90);
    p.setPen(QPen(rim, 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(0.6, 0.6, 98.8, 98.8), 26.5, 26.5);
    p.end();
    return pm;
}

} // namespace vm::ui::icons
