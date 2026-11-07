#include "home.h"
#include "../config.h"
#include "../i18n.h"
#include "../theme.h"
#include "art.h"
#include "icons.h"
#include <QBrush>
#include <QEasingCurve>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace vm::ui {

QPainterPath arch_path(const QRectF& r, double radius) {
    double w = r.width();
    double shoulder = r.top() + w * 0.56;
    double cx = r.center().x();
    QPainterPath p(QPointF(r.left(), r.bottom() - radius));
    p.lineTo(r.left(), shoulder);
    p.cubicTo(r.left(), shoulder - w * 0.30, cx - w * 0.20, r.top() + w * 0.12, cx, r.top());
    p.cubicTo(cx + w * 0.20, r.top() + w * 0.12, r.right(), shoulder - w * 0.30, r.right(), shoulder);
    p.lineTo(r.right(), r.bottom() - radius);
    p.quadTo(r.right(), r.bottom(), r.right() - radius, r.bottom());
    p.lineTo(r.left() + radius, r.bottom());
    p.quadTo(r.left(), r.bottom(), r.left(), r.bottom() - radius);
    p.closeSubpath();
    return p;
}

// ---- PrayerCard ------------------------------------------------------------
PrayerCard::PrayerCard(
    const QString& key_,
    const QString& time_text_,
    const QString& state_,
    const QString& lang_,
    bool alarm_on_,
    QWidget* parent
)
    : QAbstractButton(parent), key(key_), time_text(time_text_), state(state_), lang(lang_), alarm_on(alarm_on_) {
    setMinimumSize(118, 250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_Hover);
    setCursor(Qt::PointingHandCursor);
    m_rerender = new QTimer(this);
    m_rerender->setSingleShot(true);
    m_rerender->setInterval(140);
    connect(m_rerender, &QTimer::timeout, this, &PrayerCard::render_now);
}

void PrayerCard::set_glow(double v) {
    m_glow = v;
    if (state == QStringLiteral("next")) {
        update();
    }
}

void PrayerCard::enterEvent(QEnterEvent* e) {
    hv(1.0);
    QAbstractButton::enterEvent(e);
}

void PrayerCard::leaveEvent(QEvent* e) {
    hv(0.0);
    QAbstractButton::leaveEvent(e);
}

void PrayerCard::hv(double v) {
    if (m_ha) {
        m_ha->stop();
        m_ha->deleteLater();
    }
    m_ha = new QVariantAnimation(this);
    m_ha->setStartValue(m_h);
    m_ha->setEndValue(v);
    m_ha->setDuration(240);
    m_ha->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_ha, &QVariantAnimation::valueChanged, this, [this](const QVariant& x) {
        m_h = x.toDouble();
        update();
    });
    m_ha->start();
}

void PrayerCard::resizeEvent(QResizeEvent* e) {
    m_rerender->start();
    QAbstractButton::resizeEvent(e);
}

QRectF PrayerCard::art_rect() const {
    return QRectF(rect()).adjusted(8, 18, -8, -14);
}

QRectF PrayerCard::bell_rect() const {
    QRectF r = art_rect();
    return QRectF(r.center().x() - 16.0, r.top() + r.width() * 0.50 + 34.0, 32.0, 32.0);
}

void PrayerCard::mouseMoveEvent(QMouseEvent* e) {
    setCursor(Qt::PointingHandCursor);
    QAbstractButton::mouseMoveEvent(e);
}

void PrayerCard::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && bell_rect().contains(e->position()) && rect().contains(e->position().toPoint())) {
        alarm_on = !alarm_on;
        update();
        emit alarmToggled(key);
        setDown(false);
        return;
    }
    QAbstractButton::mouseReleaseEvent(e);
}

void PrayerCard::render_now() {
    QRectF r = art_rect();
    double dpr = devicePixelRatioF();
    m_pm = art::render(key, std::max(60, static_cast<int>(r.width())), std::max(120, static_cast<int>(r.height())), dpr);
    update();
}

void PrayerCard::paintEvent(QPaintEvent*) {
    if (m_pm.isNull()) {
        render_now();
    }
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRectF base = art_rect();
    double lift = 9.0 * m_h;
    QRectF r = base.translated(0, -lift);
    double grow = 5.0 * m_h;
    r = r.adjusted(-grow / 2.0, -grow / 2.0, grow / 2.0, grow / 2.0);
    bool nxt = (state == QStringLiteral("next"));

    // shadow
    QColor sh(0, 0, 0);
    for (int i = 0; i < 10; ++i) {
        double a = std::min(1.0, (T.dark() ? 0.22 : 0.075) * std::pow(1.0 - i / 10.0, 1.6) * (1.0 + m_h * 0.7));
        sh.setAlphaF(static_cast<float>(a));
        p.setPen(Qt::NoPen);
        p.setBrush(sh);
        p.drawPath(arch_path(r.adjusted(-i * 1.3, i * 1.2 + 4, i * 1.3, i * 2.2 + 6), 26 + i));
    }

    if (nxt) {
        QColor g(T.c(QStringLiteral("accent")));
        for (int i = 0; i < 8; ++i) {
            double a = ((0.16 + 0.10 * m_glow) * (1.0 - i / 8.0)) * (T.dark() ? 1.0 : 0.7);
            g.setAlphaF(static_cast<float>(a));
            p.setBrush(g);
            p.drawPath(arch_path(r.adjusted(-i * 1.8, -i * 1.8, i * 1.8, i * 1.8), 26 + i * 2));
        }
    }

    QPainterPath path = arch_path(r, 26);
    p.save();
    p.setClipPath(path);
    if (!m_pm.isNull()) {
        p.drawPixmap(r, m_pm, QRectF(0, 0, m_pm.width(), m_pm.height()));
    }
    // legibility gradient
    QLinearGradient g(0, r.top() + r.height() * 0.52, 0, r.bottom());
    g.setColorAt(0.00, QColor(0, 0, 0, 0));
    g.setColorAt(0.55, QColor(0, 0, 0, 120));
    g.setColorAt(1.00, QColor(0, 0, 0, 215));
    p.fillRect(r, QBrush(g));

    // top sheen
    QLinearGradient sg(0, r.top(), 0, r.top() + r.height() * 0.35);
    sg.setColorAt(0.0, QColor(255, 255, 255, 34));
    sg.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.fillRect(r, QBrush(sg));

    if (state == QStringLiteral("past")) {
        p.fillRect(r, QColor(8, 12, 10, 105));
    }
    p.restore();

    // border
    if (nxt) {
        QLinearGradient bg(r.topLeft(), r.bottomRight());
        bg.setColorAt(0, T.c(QStringLiteral("accent_hi")));
        bg.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(QPen(QBrush(bg), 2.2));
    } else {
        p.setPen(QPen(QColor(255, 255, 255, T.dark() ? 60 : 150), 1.2));
    }
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);

    // texts
    QString ar = i18n::t(key);
    QString sec = i18n::tl(key, (i18n::lang() == QStringLiteral("ar")) ? QStringLiteral("en") : QStringLiteral("ar"));
    double pad = 10.0;
    QRectF tx(r.left() + pad, r.bottom() - 112.0, r.width() - 2.0 * pad, 112.0);
    double op = (state == QStringLiteral("past")) ? 0.62 : 1.0;
    p.setOpacity(op);
    p.setPen(QColor(QStringLiteral("#FFFFFF")));
    p.setFont(ui::font(std::max(14.0, std::min(21.0, r.width() / 8.2)), 800));
    p.drawText(QRectF(tx.left(), tx.top() + 8, tx.width(), 36), Qt::AlignCenter, ar);
    p.setPen(T.c(QStringLiteral("accent_hi")));
    p.setFont(ui::font(10.5, 500));
    p.drawText(QRectF(tx.left(), tx.top() + 42, tx.width(), 20), Qt::AlignCenter, sec);
    p.setPen(QColor(QStringLiteral("#FFFFFF")));
    p.setFont(ui::font(std::max(15.0, std::min(22.0, r.width() / 7.4)), 800));
    p.drawText(QRectF(tx.left(), tx.top() + 64, tx.width(), 38), Qt::AlignCenter, time_text);
    p.setOpacity(1.0);

    // alarm bell
    QRectF br0 = bell_rect().translated(0, -lift);
    p.setPen(QPen(QColor(255, 255, 255, 70), 1));
    p.setBrush(QColor(0, 0, 0, alarm_on ? 105 : 140));
    p.drawEllipse(br0);
    icons::draw(&p, alarm_on ? QStringLiteral("bell") : QStringLiteral("bell_off"),
                br0.adjusted(7, 7, -7, -7),
                alarm_on ? T.c(QStringLiteral("accent_hi")) : QColor(255, 255, 255, 150), 2.0);

    // badge
    if (nxt) {
        QString txt = i18n::t(QStringLiteral("upcoming"));
        QFontMetrics fm(ui::font(9.0, 800));
        double bw = fm.horizontalAdvance(txt) + 26.0;
        QRectF br(r.center().x() - bw / 2.0, r.top() + r.width() * 0.50, bw, 24.0);
        QLinearGradient gg(br.topLeft(), br.bottomRight());
        gg.setColorAt(0, T.c(QStringLiteral("accent_hi")));
        gg.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(gg));
        p.drawRoundedRect(br, 12, 12);
        p.setPen(T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")));
        p.setFont(ui::font(9.0, 800));
        p.drawText(br, Qt::AlignCenter, txt);
    } else if (state == QStringLiteral("past")) {
        QRectF cr(r.center().x() - 15.0, r.top() + r.width() * 0.50, 30.0, 30.0);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 120));
        p.drawEllipse(cr);
        icons::draw(&p, QStringLiteral("check"), cr.adjusted(7, 7, -7, -7), QColor(QStringLiteral("#5BC49A")), 2.6);
    }
}

// ---- GlassChip -------------------------------------------------------------
GlassChip::GlassChip(const QString& icon, double size, int weight, bool accent_, QWidget* parent)
    : QWidget(parent), icon_name(icon), fs(size), fw(weight), accent(accent_) {
    setFixedHeight(static_cast<int>(size * 1.333) + 20);
}

void GlassChip::set_text(const QString& txt) {
    text = txt;
    setFixedWidth(QFontMetrics(ui::font(fs, fw)).horizontalAdvance(text) + 34 + (!icon_name.isEmpty() ? 24 : 0));
    update();
}

void GlassChip::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QColor fg;
    if (accent) {
        QLinearGradient g(r.topLeft(), r.bottomRight());
        g.setColorAt(0, T.c(QStringLiteral("accent_hi")));
        g.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(g));
        fg = T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF"));
    } else {
        p.setPen(QPen(QColor(255, 255, 255, 80), 1));
        p.setBrush(QColor(255, 255, 255, 36));
        fg = QColor(QStringLiteral("#FFFFFF"));
    }
    p.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);
    p.setPen(fg);
    p.setFont(ui::font(fs, fw));
    bool rtl = isRightToLeft();
    QRectF tr = r;
    if (!icon_name.isEmpty()) {
        double ix = rtl ? (r.right() - 28.0) : (r.left() + 12.0);
        icons::draw(&p, icon_name, QRectF(ix, r.center().y() - 8.0, 16.0, 16.0), fg, 2.0);
        tr = rtl ? QRectF(r.left() + 12, 0, r.width() - 42, r.height()) : QRectF(r.left() + 32, 0, r.width() - 42, r.height());
    }
    p.drawText(tr, Qt::AlignCenter, text);
}

// ---- CountdownBlock --------------------------------------------------------
CountdownBlock::CountdownBlock(QWidget* parent) : QWidget(parent) {
    setFixedSize(W, H);
}

void CountdownBlock::set_values(std::tuple<int, int, int> hms_, double frac_, const QString& cap, const QString& prev, const QString& next) {
    hms = hms_;
    frac = std::max(0.0, std::min(1.0, frac_));
    caption = cap;
    prev_name = prev;
    next_name = next;
    update();
}

void CountdownBlock::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    double w = W;
    p.setPen(QColor(255, 255, 255, 215));
    p.setFont(ui::font(10.5, 700));
    p.drawText(QRectF(0, 0, w, 22), Qt::AlignCenter, caption);

    double tw = 102, th = 88, gap = 15;
    double x0 = (w - (3 * tw + 2 * gap)) / 2.0;
    const QString labels[] = {i18n::t(QStringLiteral("unit_h")), i18n::t(QStringLiteral("unit_m")), i18n::t(QStringLiteral("unit_s"))};
    QFont f = ui::font(34.0, 800);
    QFontMetrics fm(f);
    double cell = fm.horizontalAdvance(QStringLiteral("0")) + 1.0;

    QColor acc_hi = T.c(QStringLiteral("accent_hi"));
    QColor acc = T.c(QStringLiteral("accent"));
    QColor acc_lo = T.c(QStringLiteral("accent2"));
    QColor deep_hi(acc_lo);
    QColor deep_lo = QColor(acc_lo).darker(155);

    const int vals[] = {std::get<0>(hms), std::get<1>(hms), std::get<2>(hms)};
    for (int i = 0; i < 3; ++i) {
        int val = vals[i];
        QRectF r(x0 + i * (tw + gap), 30, tw, th);
        QColor digit, label_col;
        if (i == 0) {
            QColor tint(acc);
            tint.setAlpha(46);
            QLinearGradient sheet(r.topLeft(), r.bottomLeft());
            sheet.setColorAt(0, QColor(255, 255, 255, 58));
            sheet.setColorAt(1, QColor(255, 255, 255, 20));
            p.setPen(QPen(T.c(QStringLiteral("accent_hi"), 0.55), 1));
            p.setBrush(QBrush(sheet));
            p.drawRoundedRect(r, 20, 20);
            p.setPen(Qt::NoPen);
            p.setBrush(tint);
            p.drawRoundedRect(r, 20, 20);
            digit = QColor(QStringLiteral("#FFFFFF"));
            label_col = QColor(255, 255, 255, 195);
        } else {
            QColor hi_c = (i == 1) ? acc_hi : deep_hi;
            QColor lo_c = (i == 1) ? acc_lo : deep_lo;
            QLinearGradient g(r.topLeft(), r.bottomRight());
            g.setColorAt(0, hi_c);
            g.setColorAt(1, lo_c);
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(g));
            p.drawRoundedRect(r, 20, 20);
            QColor mid((hi_c.red() + lo_c.red()) / 2, (hi_c.green() + lo_c.green()) / 2, (hi_c.blue() + lo_c.blue()) / 2);
            digit = theme::Theme::on_color(mid);
            label_col = digit;
            label_col.setAlpha(190);
        }
        p.setFont(f);
        p.setPen(digit);
        QString txt = QStringLiteral("%1").arg(val, 2, 10, QLatin1Char('0'));
        double tot = cell * txt.length();
        double x = r.center().x() - tot / 2.0;
        for (QChar ch : txt) {
            p.drawText(QRectF(x, r.top() + 10, cell, 46), Qt::AlignCenter, QString(ch));
            x += cell;
        }
        p.setFont(ui::font(8.5, 700));
        p.setPen(label_col);
        p.drawText(QRectF(r.left(), r.bottom() - 26, r.width(), 20), Qt::AlignCenter, labels[i]);
        if (i < 2) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 150));
            double cx = r.right() + gap / 2.0;
            p.drawEllipse(QPointF(cx, r.center().y() - 7), 2, 2);
            p.drawEllipse(QPointF(cx, r.center().y() + 7), 2, 2);
        }
    }

    // progress
    double by = 30 + th + 20;
    QRectF bar(x0, by, 3 * tw + 2 * gap, 8);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 52));
    p.drawRoundedRect(bar, 4, 4);
    bool rtl = isRightToLeft();
    double fw = std::max(8.0, bar.width() * frac);
    QRectF fr(rtl ? (bar.right() - fw) : bar.left(), bar.top(), fw, bar.height());
    QLinearGradient g(fr.left(), 0, fr.right(), 0);
    g.setColorAt(0, T.c(QStringLiteral("accent2")));
    g.setColorAt(1, T.c(QStringLiteral("accent_hi")));
    p.setBrush(QBrush(g));
    p.drawRoundedRect(fr, 4, 4);

    double kx = rtl ? fr.left() : fr.right();
    p.setBrush(QColor(255, 255, 255, 60));
    p.drawEllipse(QPointF(kx, bar.center().y()), 10, 10);
    p.setBrush(QColor(QStringLiteral("#FFFFFF")));
    p.drawEllipse(QPointF(kx, bar.center().y()), 5, 5);

    p.setFont(ui::font(9.0, 600));
    p.setPen(QColor(255, 255, 255, 170));
    QRectF left_r(bar.left(), by + 14, bar.width() / 2.0, 18);
    QRectF right_r(bar.center().x(), by + 14, bar.width() / 2.0, 18);
    if (rtl) {
        p.drawText(right_r, Qt::AlignRight | Qt::AlignVCenter | Qt::AlignAbsolute, prev_name);
        p.drawText(left_r, Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignAbsolute, next_name);
    } else {
        p.drawText(left_r, Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignAbsolute, prev_name);
        p.drawText(right_r, Qt::AlignRight | Qt::AlignVCenter | Qt::AlignAbsolute, next_name);
    }
}

// ---- HeroCard --------------------------------------------------------------
HeroCard::HeroCard(AppState* st, QWidget* parent)
    : QFrame(parent), state(st) {
    m_rerender = new QTimer(this);
    m_rerender->setSingleShot(true);
    m_rerender->setInterval(150);
    connect(m_rerender, &QTimer::timeout, this, &HeroCard::render_now);
    setMinimumHeight(236);
    setMaximumHeight(262);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(38, 24, 34, 24);
    lay->setSpacing(24);

    auto* col = new QVBoxLayout();
    col->setSpacing(10);
    pill = new GlassChip(QStringLiteral("bell"), 10.0, 700, true);
    name = new Label(QString(), 48.0, 800, QStringLiteral("#FFFFFF"), nullptr, Qt::AlignLeading | Qt::AlignVCenter);
    chip = new GlassChip(QStringLiteral("clock"), 13.0, 700);

    auto* row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(chip);
    row->addStretch(1);

    auto* prow = new QHBoxLayout();
    prow->setContentsMargins(0, 0, 0, 0);
    prow->addWidget(pill);
    prow->addStretch(1);

    col->addStretch(1);
    col->addLayout(prow);
    col->addWidget(name);
    col->addLayout(row);
    col->addStretch(1);
    lay->addLayout(col, 1);

    countdown = new CountdownBlock();
    lay->addWidget(countdown, 0, Qt::AlignVCenter);
}

void HeroCard::set_prayer(const QString& key_, const QString& time_text) {
    if (key_ != key || m_pm.isNull()) {
        key = key_;
        render_now();
    }
    pill->set_text(i18n::t(QStringLiteral("next_prayer")));
    name->setText(i18n::t(key));
    QString mark = i18n::is_rtl() ? QStringLiteral("\u200f") : QStringLiteral("\u200e");
    QString secLang = (i18n::lang() == QStringLiteral("ar")) ? QStringLiteral("en") : QStringLiteral("ar");
    chip->set_text(QStringLiteral("%1%2   ·   %3").arg(mark, time_text, i18n::tl(key, secLang)));
    update();
}

void HeroCard::resizeEvent(QResizeEvent* e) {
    m_rerender->start();
    QFrame::resizeEvent(e);
}

void HeroCard::render_now() {
    int w = std::max(300, width());
    int h = std::max(160, height());
    m_pm = art::render(key, w, h, devicePixelRatioF(), h * 1.0, w * 0.5);
    m_pm_key = key;
    update();
}

void HeroCard::paintEvent(QPaintEvent*) {
    if (m_pm.isNull()) {
        render_now();
    }
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
    QPainterPath path;
    path.addRoundedRect(r, 30, 30);

    if (T.dark()) {
        for (int i = 0; i < 6; ++i) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, static_cast<int>(26 * (1.0 - i / 6.0))));
            p.drawRoundedRect(r.adjusted(-i, -i + 3, i, i + 5), 30 + i, 30 + i);
        }
    }

    p.save();
    p.setClipPath(path);
    if (!m_pm.isNull()) {
        p.drawPixmap(r, m_pm, QRectF(0, 0, m_pm.width(), m_pm.height()));
    }
    bool rtl = isRightToLeft();
    double lead = rtl ? r.right() : r.left();
    double trail = rtl ? r.left() : r.right();

    QLinearGradient g(lead, 0, trail, 0);
    g.setColorAt(0.00, QColor(6, 8, 14, 190));
    g.setColorAt(0.38, QColor(6, 8, 14, 95));
    g.setColorAt(0.62, QColor(6, 8, 14, 0));
    p.fillRect(r, QBrush(g));

    QLinearGradient g2(trail, 0, lead, 0);
    g2.setColorAt(0.00, QColor(6, 8, 14, 165));
    g2.setColorAt(0.36, QColor(6, 8, 14, 70));
    g2.setColorAt(0.60, QColor(6, 8, 14, 0));
    p.fillRect(r, QBrush(g2));

    QLinearGradient sh(0, r.top(), 0, r.top() + 60);
    sh.setColorAt(0.0, QColor(255, 255, 255, 34));
    sh.setColorAt(1.0, QColor(255, 255, 255, 0));
    p.fillRect(r, QBrush(sh));
    p.restore();

    QLinearGradient bp(r.topLeft(), r.bottomRight());
    bp.setColorAt(0, T.c(QStringLiteral("accent_hi"), 0.75));
    bp.setColorAt(1, T.c(QStringLiteral("accent2"), 0.25));
    p.setPen(QPen(QBrush(bp), 1.4));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, 30, 30);
}

// ---- QLabelIcon ------------------------------------------------------------
QLabelIcon::QLabelIcon(const QString& name_, int size, const QString& color_, QWidget* parent)
    : QWidget(parent), name(name_), color(color_) {
    setFixedSize(size, size);
}

void QLabelIcon::paintEvent(QPaintEvent*) {
    QPainter p(this);
    icons::draw(&p, name, QRectF(rect()), theme::T().c(color), 2.0);
}

// ---- HomePage --------------------------------------------------------------
HomePage::HomePage(AppState* st, Notifier* notif, QWidget* parent)
    : QWidget(parent), state(st), notifier(notif) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(30, 14, 30, 22);
    root->setSpacing(18);

    // header
    auto* head = new QHBoxLayout();
    head->setSpacing(12);
    auto* col = new QVBoxLayout();
    col->setSpacing(2);
    auto* top = new QHBoxLayout();
    top->setSpacing(10);
    pin = new QLabelIcon(QStringLiteral("pin"), 22);
    top->addWidget(pin);
    city = new Label(QString(), 27.0, 800, QStringLiteral("text"));
    top->addWidget(city);
    top->addStretch();
    col->addLayout(top);
    dateline = new Label(QString(), 11.5, 500, QStringLiteral("text2"), nullptr, Qt::AlignLeading | Qt::AlignVCenter);
    col->addWidget(dateline);
    head->addLayout(col, 1);

    snooze_chip = new Button(QString(), QStringLiteral("ghost"), QStringLiteral("snooze"), nullptr, 40);
    snooze_chip->hide();
    connect(snooze_chip, &QAbstractButton::clicked, this, &HomePage::snoozeCancel);
    head->addWidget(snooze_chip);

    lang = new Segmented({{QStringLiteral("ar"), QStringLiteral("عربي")}, {QStringLiteral("en"), QStringLiteral("EN")}},
                         i18n::lang(), nullptr, {}, 40, 58);
    connect(lang, &Segmented::changed, this, &HomePage::languageChanged);
    head->addWidget(lang);

    theme_btn = new IconButton(QStringLiteral("sun"), 40, QString(), nullptr, 14);
    connect(theme_btn, &QAbstractButton::clicked, this, &HomePage::themeToggle);
    head->addWidget(theme_btn);
    root->addLayout(head);

    hero = new HeroCard(state);
    root->addWidget(hero);

    scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    row_host = new QWidget();
    row = new QHBoxLayout(row_host);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);
    scroll->setWidget(row_host);
    root->addWidget(scroll, 1);

    m_glow_a = new QVariantAnimation(this);
    m_glow_a->setStartValue(0.0);
    m_glow_a->setEndValue(1.0);
    m_glow_a->setDuration(2600);
    m_glow_a->setLoopCount(-1);
    m_glow_a->setEasingCurve(QEasingCurve::InOutSine);
    connect(m_glow_a, &QVariantAnimation::valueChanged, this, &HomePage::on_glow);
    m_glow_a->start();

    connect(state, &AppState::alertsChanged, this, &HomePage::sync_bells);
    refresh();
}

void HomePage::sync_bells() {
    for (PrayerCard* c : m_cards) {
        c->alarm_on = state->s.alerts.value(c->key, true);
        c->update();
    }
}

void HomePage::toggle_alarm(const QString& key) {
    auto& s = state->s;
    s.alerts[key] = !s.alerts.value(key, true);
    s.save();
    emit state->toast(
        (s.alerts[key] ? QStringLiteral("alarm_on|") : QStringLiteral("alarm_off|")) + key,
        s.alerts[key] ? QStringLiteral("ok") : QStringLiteral("warn")
    );
    emit state->alertsChanged();
}

void HomePage::on_glow(const QVariant& v) {
    for (PrayerCard* c : m_cards) {
        c->set_glow(v.toDouble());
    }
}

void HomePage::restyle() {
    city->restyle();
    dateline->restyle();
    pin->update();
    theme_btn->set_icon(theme::T().dark() ? QStringLiteral("sun") : QStringLiteral("moon"));
}

void HomePage::refresh() {
    auto snap = state->snapshot();
    auto c = state->country();
    auto r = state->region();
    city->setText((i18n::lang() == QStringLiteral("ar")) ? r.ar : r.en);
    QDateTime now = state->now();
    dateline->setText(QStringLiteral("%1   ·   %2").arg(c.name(i18n::lang()), i18n::fmt_date(now.date(), state->hijri(now.date()))));
    theme_btn->set_icon(theme::T().dark() ? QStringLiteral("sun") : QStringLiteral("moon"));
    lang->set_current(i18n::lang(), false);

    QMap<QString, QString> tm;
    for (const auto& [k, dt] : snap.today) {
        tm[k] = dt.toString(QStringLiteral("HH:mm"));
    }
    hero->set_prayer(snap.next_key, snap.next_dt.toString(QStringLiteral("HH:mm")));

    while (QLayoutItem* it = row->takeAt(0)) {
        if (QWidget* w = it->widget()) {
            w->hide();
            w->setParent(nullptr);
            w->deleteLater();
        }
        delete it;
    }
    m_cards.clear();
    for (const QString& k : PRAYER_KEYS) {
        QString st = (k == snap.next_key) ? QStringLiteral("next")
                                          : (snap.past.contains(k) ? QStringLiteral("past") : QStringLiteral("future"));
        auto* card = new PrayerCard(k, tm.value(k), st, i18n::lang(), state->s.alerts.value(k, true));
        connect(card, &PrayerCard::alarmToggled, this, &HomePage::toggle_alarm);
        m_cards.append(card);
        row->addWidget(card, 1);
    }
    m_last_next = snap.next_key;
    tick();
}

void HomePage::tick() {
    auto snap = state->snapshot();
    QDateTime now = state->now();
    qint64 secs = std::max<qint64>(0, now.secsTo(snap.next_dt));
    int h = static_cast<int>(secs / 3600);
    int rem = static_cast<int>(secs % 3600);
    int m = rem / 60;
    int s = rem % 60;
    double total = std::max(1.0, static_cast<double>(snap.prev_dt.secsTo(snap.next_dt)));
    double frac = 1.0 - secs / total;
    hero->countdown->set_values(
        {h, m, s},
        frac,
        i18n::t(QStringLiteral("until"), {{QStringLiteral("name"), i18n::t(snap.next_key)}}),
        i18n::t(snap.prev_key),
        i18n::t(snap.next_key)
    );
    if (snap.next_key != m_last_next) {
        refresh();
    }
    auto sn = notifier->next_snooze();
    if (sn.has_value()) {
        qint64 left = std::max<qint64>(0, now.secsTo(sn->second));
        snooze_chip->setText(QStringLiteral("%1 · %2:%3")
                                 .arg(i18n::t(QStringLiteral("snoozed")))
                                 .arg(left / 60, 2, 10, QLatin1Char('0'))
                                 .arg(left % 60, 2, 10, QLatin1Char('0')));
        snooze_chip->setFixedWidth(std::max(130, snooze_chip->sizeHint().width()));
        snooze_chip->show();
    } else {
        snooze_chip->hide();
    }
}

} // namespace vm::ui
