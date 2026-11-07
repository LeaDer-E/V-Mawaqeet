#include "widgets.h"
#include "../theme.h"
#include "icons.h"
#include <QBrush>
#include <QEasingCurve>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QScreen>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace vm::ui {

QFont font(double size, int weight) {
    QFont f(FAMILY);
    f.setPixelSize(static_cast<int>(std::llround(size * 1.333)));
    f.setWeight(static_cast<QFont::Weight>(weight));
    return f;
}

static QVariantAnimation* make_anim(double target, double start, int ms, QObject* parent, std::function<void(double)> cb) {
    auto* a = new QVariantAnimation(parent);
    a->setStartValue(start);
    a->setEndValue(target);
    a->setDuration(ms);
    a->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(a, &QVariantAnimation::valueChanged, parent, [cb = std::move(cb)](const QVariant& val) {
        cb(val.toDouble());
    });
    return a;
}

// ---- Card ------------------------------------------------------------------
Card::Card(int rad, QWidget* parent, int pad)
    : QFrame(parent), radius(rad) {
    setAttribute(Qt::WA_StyledBackground, false);
    if (pad > 0) {
        setContentsMargins(pad, pad, pad, pad);
    }
}

void Card::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    if (!T.dark()) {
        const int alphas[] = {10, 7, 4};
        for (int i = 0; i < 3; ++i) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(60, 45, 15, alphas[i]));
            p.drawRoundedRect(r.adjusted(-i * 1.2, i * 0.4, i * 1.2, i * 2.4 + 1), radius + i, radius + i);
        }
    }
    p.setPen(QPen(T.c(QStringLiteral("border")), 1));
    p.setBrush(T.c(QStringLiteral("surface")));
    p.drawRoundedRect(r, radius, radius);
}

// ---- Label -----------------------------------------------------------------
Label::Label(const QString& text, double size, int weight, const QString& color, QWidget* parent, std::optional<Qt::Alignment> align)
    : QLabel(text, parent), m_color(color), m_size(size), m_weight(weight) {
    setFont(ui::font(size, weight));
    setAttribute(Qt::WA_TranslucentBackground);
    if (align.has_value()) {
        setAlignment(*align);
    }
    restyle();
}

void Label::restyle() {
    auto& T = theme::T();
    QString css = (m_color.startsWith(QLatin1Char('#')) || m_color.startsWith(QStringLiteral("rgba")))
                      ? m_color
                      : T.css(m_color);
    setStyleSheet(QStringLiteral("color:%1; background:transparent;").arg(css));
}

void Label::set_color(const QString& c) {
    m_color = c;
    restyle();
}

// ---- Button ----------------------------------------------------------------
Button::Button(const QString& text, const QString& kind_, const QString& icon, QWidget* parent, int height)
    : QAbstractButton(parent), kind(kind_), icon_name(icon) {
    setText(text);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(height);
    setFont(ui::font(11.0, 700));
    setAttribute(Qt::WA_Hover);
    m_timer = new QTimer(this);
    m_timer->setInterval(16);
    connect(m_timer, &QTimer::timeout, this, &Button::tick);
}

QSize Button::sizeHint() const {
    QFontMetrics fm(ui::font(11.0, 700));
    int w = fm.horizontalAdvance(text()) + 40 + ((!icon_name.isEmpty() || m_busy) ? 26 : 0);
    return QSize(std::max(w, 64), height());
}

void Button::set_busy(bool b) {
    m_busy = b;
    setEnabled(!b);
    if (b) m_timer->start();
    else m_timer->stop();
    update();
}

void Button::tick() {
    m_spin = std::fmod(m_spin + 9.0, 360.0);
    update();
}

void Button::hover_to(double v) {
    if (m_ha) {
        m_ha->stop();
        m_ha->deleteLater();
    }
    m_ha = make_anim(v, m_h, 160, this, [this](double x) {
        m_h = x;
        update();
    });
    m_ha->start();
}

void Button::enterEvent(QEnterEvent* e) {
    hover_to(1.0);
    QAbstractButton::enterEvent(e);
}

void Button::leaveEvent(QEvent* e) {
    hover_to(0.0);
    QAbstractButton::leaveEvent(e);
}

void Button::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    double rad = 13.0;
    bool down = isDown();
    QColor fg;

    if (kind == QStringLiteral("primary")) {
        QLinearGradient g(0, 0, r.width(), r.height());
        QColor a = T.c(QStringLiteral("accent"));
        QColor b = T.c(QStringLiteral("accent2"));
        int lift = static_cast<int>(100.0 * (1.0 + 0.07 * m_h));
        g.setColorAt(0, a.lighter(lift));
        g.setColorAt(1, b.lighter(lift));
        if (T.dark() && m_h > 0.01) {
            QColor glow = T.c(QStringLiteral("accent"));
            glow.setAlphaF(static_cast<float>(0.20 * m_h));
            p.setPen(Qt::NoPen);
            p.setBrush(glow);
            p.drawRoundedRect(r.adjusted(-2, 1, 2, 3), rad + 2, rad + 2);
        }
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(g));
        fg = T.c(QStringLiteral("on_accent"));
    } else if (kind == QStringLiteral("glass")) {
        p.setPen(QPen(QColor(255, 255, 255, static_cast<int>(70 + 70 * m_h)), 1));
        p.setBrush(QColor(255, 255, 255, static_cast<int>(34 + 40 * m_h)));
        fg = QColor(QStringLiteral("#FFFFFF"));
    } else if (kind == QStringLiteral("danger")) {
        QColor c = T.c(QStringLiteral("danger"));
        c.setAlphaF(static_cast<float>(0.12 + 0.12 * m_h));
        p.setPen(QPen(T.c(QStringLiteral("danger"), 0.5), 1));
        p.setBrush(c);
        fg = T.c(QStringLiteral("danger"));
    } else {
        QColor c = (kind == QStringLiteral("ghost"))
                       ? (T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(27, 36, 32, 14))
                       : QColor(0, 0, 0, 0);
        if (m_h > 0.0 && kind == QStringLiteral("plain")) {
            c = T.dark() ? QColor(255, 255, 255, static_cast<int>(24 * m_h))
                         : QColor(27, 36, 32, static_cast<int>(16 * m_h));
        }
        p.setPen((kind == QStringLiteral("ghost")) ? QPen(T.c(QStringLiteral("border_hi")), 1) : Qt::NoPen);
        p.setBrush(c);
        fg = T.c(QStringLiteral("text"));
        if (kind == QStringLiteral("ghost") && m_h > 0.0) {
            p.setPen(QPen(T.c(QStringLiteral("accent"), 0.35 + 0.65 * m_h), 1));
        }
    }

    if (down) {
        p.translate(0, 1);
    }
    if (!isEnabled() && !m_busy) {
        p.setOpacity(0.45);
    }
    p.drawRoundedRect(r, rad, rad);
    p.setPen(fg);
    p.setFont(this->font());

    QFontMetrics fm(this->font());
    int tw = fm.horizontalAdvance(text());
    bool has_icon = !icon_name.isEmpty() || m_busy;
    int total = tw + ((has_icon && !text().isEmpty()) ? 22 : (has_icon ? 18 : 0));
    double x0 = (width() - total) / 2.0;
    bool rtl = isRightToLeft();
    QRectF icon_rect(0, (height() - 18.0) / 2.0, 18.0, 18.0);
    QRectF text_rect(0, 0, tw, height());
    if (rtl) {
        icon_rect.moveLeft(x0 + tw + (!text().isEmpty() ? 4 : 0));
        text_rect.moveLeft(x0);
    } else {
        icon_rect.moveLeft(x0);
        text_rect.moveLeft(x0 + ((has_icon && !text().isEmpty()) ? 22 : (has_icon ? 18 : 0)));
    }
    if (m_busy) {
        QPen pen(fg, 2.2);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.drawArc(icon_rect.adjusted(2, 2, -2, -2), static_cast<int>(-m_spin * 16), 100 * 16);
        p.setPen(fg);
    } else if (!icon_name.isEmpty()) {
        icons::draw(&p, icon_name, icon_rect, fg, 1.9);
    }
    if (!text().isEmpty()) {
        p.drawText(text_rect, Qt::AlignCenter, text());
    }
}

// ---- IconButton ------------------------------------------------------------
IconButton::IconButton(const QString& icon, int size, const QString& tip, QWidget* parent, int radius_, const QString& color, int glyph_)
    : QAbstractButton(parent), icon_name(icon), radius(radius_), color_name(color), glyph(glyph_) {
    setFixedSize(size, size);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    if (!tip.isEmpty()) {
        setToolTip(tip);
    }
}

void IconButton::set_icon(const QString& name) {
    icon_name = name;
    update();
}

void IconButton::hover_to(double v) {
    if (m_ha) {
        m_ha->stop();
        m_ha->deleteLater();
    }
    m_ha = make_anim(v, m_h, 160, this, [this](double x) {
        m_h = x;
        update();
    });
    m_ha->start();
}

void IconButton::enterEvent(QEnterEvent* e) {
    hover_to(1.0);
    QAbstractButton::enterEvent(e);
}

void IconButton::leaveEvent(QEvent* e) {
    hover_to(0.0);
    QAbstractButton::leaveEvent(e);
}

void IconButton::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    if (danger && m_h > 0.0) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(229, 70, 60, static_cast<int>(235 * m_h)));
    } else if (fixed_color.has_value()) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, static_cast<int>(40 + 50 * m_h)));
    } else {
        QColor bcol = T.c(QStringLiteral("border_hi"));
        bcol.setAlphaF(bcol.alphaF() * static_cast<float>(m_h));
        QColor scol = T.dark() ? QColor(255, 255, 255, static_cast<int>(28 * m_h))
                               : QColor(27, 36, 32, static_cast<int>(16 * m_h));
        p.setPen(QPen(bcol, 1));
        p.setBrush(scol);
    }
    p.drawRoundedRect(r, radius, radius);
    QColor col = (danger && m_h > 0.5)
                     ? QColor(QStringLiteral("#FFFFFF"))
                     : ((m_h > 0.4) ? T.c(QStringLiteral("text")) : T.c(color_name));
    if (fixed_color.has_value()) {
        col = *fixed_color;
    }
    QRectF g(0, 0, glyph, glyph);
    g.moveCenter(QRectF(rect()).center());
    icons::draw(&p, icon_name, g, col, 1.8);
}

// ---- Toggle ----------------------------------------------------------------
Toggle::Toggle(bool checked, QWidget* parent)
    : QAbstractButton(parent), m_x(checked ? 1.0 : 0.0) {
    setCheckable(true);
    setChecked(checked);
    setFixedSize(50, 28);
    setCursor(Qt::PointingHandCursor);
    connect(this, &QAbstractButton::clicked, this, &Toggle::on_clicked);
}

void Toggle::on_clicked() {
    go(isChecked() ? 1.0 : 0.0);
    emit toggled_by_user(isChecked());
}

void Toggle::set_value(bool v) {
    setChecked(v);
    m_x = v ? 1.0 : 0.0;
    update();
}

void Toggle::go(double v) {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = make_anim(v, m_x, 200, this, [this](double x) {
        m_x = x;
        update();
    });
    m_a->setEasingCurve(QEasingCurve::OutBack);
    m_a->start();
}

void Toggle::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
    double t = std::max(0.0, std::min(1.0, m_x));
    QColor off = !T.dark() ? T.c(QStringLiteral("text3"), 0.45) : T.c(QStringLiteral("border_hi"));
    QColor on = T.c(QStringLiteral("accent"));
    QColor mix(
        static_cast<int>(off.red() + (on.red() - off.red()) * t),
        static_cast<int>(off.green() + (on.green() - off.green()) * t),
        static_cast<int>(off.blue() + (on.blue() - off.blue()) * t),
        static_cast<int>(off.alpha() + (255 - off.alpha()) * t)
    );
    p.setPen(Qt::NoPen);
    p.setBrush(mix);
    p.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);
    double d = r.height() - 6.0;
    bool rtl = isRightToLeft();
    double tt = rtl ? (1.0 - t) : t;
    double x = r.left() + 3.0 + (r.width() - d - 6.0) * tt;
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawEllipse(QRectF(x, r.top() + 3.8, d, d));
    p.setBrush((t > 0.5 || !T.dark()) ? QColor(QStringLiteral("#FFFFFF")) : QColor(QStringLiteral("#D9E2DD")));
    p.drawEllipse(QRectF(x, r.top() + 3.0, d, d));
}

// ---- Segmented -------------------------------------------------------------
Segmented::Segmented(
    const QVector<std::pair<QString, QString>>& items_,
    const QString& current_,
    QWidget* parent,
    const QMap<QString, QString>& icons_,
    int height,
    int min_seg
)
    : QWidget(parent), items(items_), current(current_), icons_map(icons_), m_min_seg(min_seg) {
    setFixedHeight(height);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    m_pos = static_cast<double>(current_index());
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

int Segmented::current_index() const {
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].first == current) return i;
    }
    return 0;
}

QSize Segmented::sizeHint() const {
    QFontMetrics fm(ui::font(10.5, 700));
    int maxAdv = 0;
    for (const auto& [k, lbl] : items) {
        maxAdv = std::max(maxAdv, fm.horizontalAdvance(lbl));
    }
    int seg = std::max(m_min_seg, maxAdv + 44 + (!icons_map.isEmpty() ? 22 : 0));
    return QSize(seg * static_cast<int>(items.size()) + 8, height());
}

void Segmented::set_current(const QString& key, bool animate) {
    if (key == current) return;
    current = key;
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
        m_a = nullptr;
    }
    if (animate) {
        m_a = make_anim(static_cast<double>(current_index()), m_pos, 240, this, [this](double v) {
            m_pos = v;
            update();
        });
        m_a->start();
    } else {
        m_pos = static_cast<double>(current_index());
        update();
    }
}

double Segmented::slot_pos(double i) const {
    int n = static_cast<int>(items.size());
    return isRightToLeft() ? (n - 1.0 - i) : i;
}

void Segmented::mouseReleaseEvent(QMouseEvent* e) {
    int n = static_cast<int>(items.size());
    if (n <= 0) return;
    double w = (width() - 8.0) / n;
    int i = static_cast<int>((e->position().x() - 4.0) / w);
    if (isRightToLeft()) i = n - 1 - i;
    if (i >= 0 && i < n && items[i].first != current) {
        set_current(items[i].first);
        emit changed(current);
    }
}

void Segmented::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(T.c(QStringLiteral("border")), 1));
    p.setBrush(T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(27, 36, 32, 14));
    p.drawRoundedRect(r, r.height() / 2.4, r.height() / 2.4);

    int n = static_cast<int>(items.size());
    if (n <= 0) return;
    double w = (width() - 8.0) / n;
    QRectF th(4.0 + slot_pos(m_pos) * w, 4.0, w, height() - 8.0);
    QLinearGradient g(th.topLeft(), th.bottomRight());
    g.setColorAt(0, T.c(QStringLiteral("accent")));
    g.setColorAt(1, T.c(QStringLiteral("accent2")));
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(g));
    p.drawRoundedRect(th, (height() - 8.0) / 2.4, (height() - 8.0) / 2.4);

    p.setFont(ui::font(10.5, 700));
    for (int i = 0; i < n; ++i) {
        const auto& [k, label] = items[i];
        QRectF cell(4.0 + slot_pos(i) * w, 4.0, w, height() - 8.0);
        double sel = std::max(0.0, 1.0 - std::abs(m_pos - i));
        QColor col = (sel > 0.5) ? T.c(QStringLiteral("on_accent")) : T.c(QStringLiteral("text2"));
        p.setPen(col);
        QString ic = icons_map.value(k);
        if (!ic.isEmpty()) {
            QFontMetrics fm(p.font());
            int tw = fm.horizontalAdvance(label);
            int tot = tw + 24;
            double x0 = cell.center().x() - tot / 2.0;
            QRectF ir(x0 + (isRightToLeft() ? (tw + 6) : 0), cell.center().y() - 8.0, 16.0, 16.0);
            QRectF tr(x0 + (isRightToLeft() ? 0 : 22), cell.top(), tw, cell.height());
            icons::draw(&p, ic, ir, col, 1.8);
            p.drawText(tr, Qt::AlignCenter, label);
        } else {
            p.drawText(cell, Qt::AlignCenter, label);
        }
    }
}

// ---- Stepper ---------------------------------------------------------------
Stepper::Stepper(int val, int lo_, int hi_, QWidget* parent)
    : QWidget(parent), value(val), lo(lo_), hi(hi_) {
    setFixedSize(118, 38);
    setCursor(Qt::PointingHandCursor);
}

void Stepper::set_value(int v) {
    value = std::max(lo, std::min(hi, v));
    update();
}

void Stepper::mouseReleaseEvent(QMouseEvent* e) {
    double x = e->position().x();
    bool rtl = isRightToLeft();
    bool left_zone = x < 36.0;
    bool right_zone = x > width() - 36.0;
    int d = 0;
    if (left_zone) d = rtl ? 1 : -1;
    else if (right_zone) d = rtl ? -1 : 1;
    if (d != 0) {
        value = std::max(lo, std::min(hi, value + d));
        update();
        emit changed(value);
    }
}

void Stepper::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(T.c(QStringLiteral("border_hi")), 1));
    p.setBrush(T.c(QStringLiteral("surface_hi")));
    p.drawRoundedRect(r, 12, 12);
    bool rtl = isRightToLeft();
    const QString names[] = {QStringLiteral("minus"), QStringLiteral("plus")};
    for (int i = 0; i < 2; ++i) {
        QRectF zone(((i == 0) != rtl) ? 0.0 : (width() - 36.0), 0.0, 36.0, height());
        icons::draw(&p, names[i], QRectF(zone.center().x() - 7.0, zone.center().y() - 7.0, 14.0, 14.0), T.c(QStringLiteral("accent")), 2.0);
    }
    p.setPen(T.c(QStringLiteral("text")));
    p.setFont(ui::font(11.0, 700));
    QString txt = (value != 0) ? QString::asprintf("%+d", value) : QStringLiteral("0");
    p.drawText(QRectF(36, 0, width() - 72, height()), Qt::AlignCenter, txt);
}

// ---- MinutesInput ----------------------------------------------------------
MinutesInput::MinutesInput(int val, int lo_, int hi_, const QString& unit, QWidget* parent, bool compact)
    : QWidget(parent), lo(lo_), hi(hi_) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(10);
    edit = new QLineEdit(QString::number(val));
    edit->setValidator(new QIntValidator(lo, hi, this));
    edit->setAlignment(Qt::AlignCenter);
    edit->setFixedSize(compact ? 64 : 84, compact ? 38 : 42);
    edit->setFont(ui::font(compact ? 11.5 : 12.0, 700));
    connect(edit, &QLineEdit::textChanged, this, &MinutesInput::on_tc);
    connect(edit, &QLineEdit::editingFinished, this, &MinutesInput::on_fin);
    lay->addWidget(edit);
    if (!unit.isEmpty()) {
        lay->addWidget(new Label(unit, 10.5, 600, QStringLiteral("text2")));
    }
}

int MinutesInput::value() const {
    bool ok = false;
    int v = edit->text().toInt(&ok);
    if (!ok) return lo;
    return std::max(lo, std::min(hi, v));
}

void MinutesInput::set_value(int v) {
    edit->setText(QString::number(v));
}

void MinutesInput::on_tc(const QString& txt) {
    bool ok = false;
    txt.trimmed().toInt(&ok);
    if (ok) {
        emit changed(value());
    }
}

void MinutesInput::on_fin() {
    edit->setText(QString::number(value()));
}

// ---- StatusBadge -----------------------------------------------------------
StatusBadge::StatusBadge(const QString& txt, const QString& knd, QWidget* parent)
    : QWidget(parent), text(txt), kind(knd) {
    setFixedHeight(26);
}

void StatusBadge::set(const QString& txt, const QString& knd) {
    text = txt;
    kind = knd;
    updateGeometry();
    update();
}

QSize StatusBadge::sizeHint() const {
    return QSize(QFontMetrics(ui::font(9.0, 700)).horizontalAdvance(text) + 34, 26);
}

void StatusBadge::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QColor col = T.c(QStringLiteral("text3"));
    if (kind == QStringLiteral("ok")) col = T.c(QStringLiteral("ok"));
    else if (kind == QStringLiteral("failed")) col = T.c(QStringLiteral("danger"));
    else if (kind == QStringLiteral("cached")) col = T.c(QStringLiteral("accent"));
    else if (kind == QStringLiteral("ready")) col = T.dark() ? T.c(QStringLiteral("emerald")) : T.c(QStringLiteral("ok"));

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QColor bg(col);
    bg.setAlphaF(0.14f);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, 13, 13);
    p.setBrush(col);
    double dx = isRightToLeft() ? (width() - 14.0) : 14.0;
    p.drawEllipse(QPointF(dx, 13.0), 3.2, 3.2);
    p.setPen(col);
    p.setFont(ui::font(9.0, 700));
    QRectF tr = isRightToLeft() ? QRectF(6, 0, width() - 26, 26) : QRectF(22, 0, width() - 26, 26);
    p.drawText(tr, Qt::AlignCenter, text);
}

// ---- SearchPopup & SearchCombo ---------------------------------------------
SearchPopup::SearchPopup(const QVector<ComboItem>& items_, const QString& placeholder, const QString& no_res, QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
      items(items_),
      no_results(no_res) {
    setAttribute(Qt::WA_TranslucentBackground);
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->setSpacing(8);
    edit = new QLineEdit();
    edit->setPlaceholderText(placeholder);
    edit->setFont(ui::font(10.5, 500));
    list = new QListWidget();
    list->setFont(ui::font(10.5, 500));
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    lay->addWidget(edit);
    lay->addWidget(list, 1);
    connect(edit, &QLineEdit::textChanged, this, &SearchPopup::fill);
    connect(list, &QListWidget::itemClicked, this, &SearchPopup::click_item);
    connect(edit, &QLineEdit::returnPressed, this, [this]() {
        click_item(list->currentItem() ? list->currentItem() : list->item(0));
    });
    setStyleSheet(theme::T().qss());
    fill(QString());
}

void SearchPopup::fill(const QString& q) {
    QString query = q.trimmed().toLower();
    list->clear();
    for (const auto& item : items) {
        if (query.isEmpty() || item.haystack.contains(query)) {
            QString text = item.label + (!item.sublabel.isEmpty() ? QStringLiteral("   ·   ") + item.sublabel : QString());
            auto* it = new QListWidgetItem(text);
            it->setData(Qt::UserRole, item.value);
            list->addItem(it);
        }
    }
    if (list->count() == 0) {
        auto* it = new QListWidgetItem(no_results);
        it->setFlags(Qt::NoItemFlags);
        list->addItem(it);
    } else if (!query.isEmpty()) {
        list->setCurrentRow(0);
    }
}

void SearchPopup::click_item(QListWidgetItem* it) {
    if (!it || !it->data(Qt::UserRole).isValid()) return;
    emit picked(it->data(Qt::UserRole));
    close();
}

void SearchPopup::select_value(const QVariant& val) {
    for (int i = 0; i < list->count(); ++i) {
        if (list->item(i)->data(Qt::UserRole) == val) {
            list->setCurrentRow(i);
            list->scrollToItem(list->item(i), QAbstractItemView::PositionAtCenter);
            break;
        }
    }
}

void SearchPopup::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
    p.setPen(QPen(T.c(QStringLiteral("border_hi")), 1));
    QColor gl = T.c(QStringLiteral("glass"));
    p.setBrush(QColor(gl.red(), gl.green(), gl.blue(), 250));
    p.drawRoundedRect(r, 16, 16);
}

SearchCombo::SearchCombo(const QString& placeholder_, const QString& no_results_, QWidget* parent)
    : QAbstractButton(parent), placeholder(placeholder_), no_results(no_results_) {
    setFixedHeight(46);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(this, &QAbstractButton::clicked, this, &SearchCombo::open_popup);
}

void SearchCombo::set_items(const QVector<ComboItem>& items_) {
    items = items_;
    update();
}

void SearchCombo::set_value(const QVariant& v) {
    value = v;
    update();
}

QString SearchCombo::current_label() const {
    for (const auto& item : items) {
        if (item.value == value) return item.label;
    }
    return QString();
}

void SearchCombo::hover_to(double v) {
    if (m_ha) {
        m_ha->stop();
        m_ha->deleteLater();
    }
    m_ha = make_anim(v, m_h, 160, this, [this](double x) {
        m_h = x;
        update();
    });
    m_ha->start();
}

void SearchCombo::enterEvent(QEnterEvent* e) {
    hover_to(1.0);
    QAbstractButton::enterEvent(e);
}

void SearchCombo::leaveEvent(QEvent* e) {
    hover_to(0.0);
    QAbstractButton::leaveEvent(e);
}

void SearchCombo::open_popup() {
    if (m_pop) {
        m_pop->deleteLater();
    }
    m_pop = new SearchPopup(items, placeholder, no_results, window());
    connect(m_pop, &SearchPopup::picked, this, &SearchCombo::on_picked);
    m_pop->setFixedWidth(std::max(width(), 280));
    m_pop->setFixedHeight(340);
    QPoint gp = mapToGlobal(QPoint(0, height() + 6));
    if (screen()) {
        QRect scr = screen()->availableGeometry();
        if (gp.y() + 340 > scr.bottom()) {
            gp = mapToGlobal(QPoint(0, -346));
        }
    }
    if (isRightToLeft()) {
        gp.setX(mapToGlobal(QPoint(width(), 0)).x() - m_pop->width());
    }
    m_pop->move(gp);
    m_pop->show();
    m_pop->edit->setFocus();
    m_pop->select_value(value);
}

void SearchCombo::on_picked(const QVariant& v) {
    value = v;
    update();
    emit changed(v);
}

void SearchCombo::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen((m_h > 0.05) ? T.c(QStringLiteral("accent"), 0.4 + 0.6 * m_h) : T.c(QStringLiteral("border_hi")), 1));
    p.setBrush(T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(QStringLiteral("#FFFFFF")));
    if (!T.dark() && m_h < 0.05) {
        p.setBrush(QColor(27, 36, 32, 10));
    }
    p.drawRoundedRect(r, 14, 14);
    p.setFont(ui::font(11.0, 500));
    p.setPen(T.c(QStringLiteral("text")));
    bool rtl = isRightToLeft();
    QString txt = current_label();
    if (txt.isEmpty()) txt = placeholder;
    QRectF tr = !rtl ? QRectF(16, 0, width() - 56, height()) : QRectF(40, 0, width() - 56, height());
    QFontMetrics fm(p.font());
    p.drawText(
        tr,
        (rtl ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter | Qt::AlignAbsolute,
        fm.elidedText(txt, Qt::ElideRight, static_cast<int>(tr.width()))
    );
    double cx = rtl ? 20.0 : (width() - 20.0);
    icons::draw(&p, QStringLiteral("chev_down"), QRectF(cx - 8.0, height() / 2.0 - 8.0, 16.0, 16.0), T.c(QStringLiteral("text2")), 2.0);
}

// ---- Toast -----------------------------------------------------------------
Toast::Toast(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    hide();
    m_fx = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_fx);
    m_t = new QTimer(this);
    m_t->setSingleShot(true);
    connect(m_t, &QTimer::timeout, this, &Toast::fade_out);
}

void Toast::show_msg(const QString& txt, const QString& knd, int ms) {
    text = txt;
    kind = knd;
    QFontMetrics fm(ui::font(10.5, 700));
    resize(fm.horizontalAdvance(text) + 76, 48);
    if (parentWidget()) {
        int pw = parentWidget()->width();
        move((pw - width()) / 2, parentWidget()->height() - 96);
    }
    raise();
    show();
    anim_to(1.0);
    m_t->start(ms);
    update();
}

void Toast::anim_to(double v) {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = new QPropertyAnimation(m_fx, "opacity", this);
    m_a->setDuration(220);
    m_a->setEndValue(v);
    m_a->setEasingCurve(QEasingCurve::OutCubic);
    if (v == 0.0) {
        connect(m_a, &QPropertyAnimation::finished, this, &QWidget::hide);
    }
    m_a->start();
}

void Toast::fade_out() {
    anim_to(0.0);
}

void Toast::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
    QColor col = (kind == QStringLiteral("ok")) ? T.c(QStringLiteral("ok")) : T.c(QStringLiteral("accent"));
    p.setPen(QPen(T.c(QStringLiteral("border_hi")), 1));
    QColor bg = T.dark() ? T.c(QStringLiteral("bg1")).lighter(115) : QColor(QStringLiteral("#FFFFFF"));
    p.setBrush(bg);
    p.drawRoundedRect(r, 24, 24);
    bool rtl = isRightToLeft();
    double cx = rtl ? (r.right() - 28.0) : (r.left() + 28.0);
    p.setPen(Qt::NoPen);
    QColor c2(col);
    c2.setAlphaF(0.18f);
    p.setBrush(c2);
    p.drawEllipse(QPointF(cx, r.center().y()), 12, 12);
    icons::draw(&p, QStringLiteral("check"), QRectF(cx - 7.0, r.center().y() - 7.0, 14.0, 14.0), col, 2.4);
    p.setPen(T.c(QStringLiteral("text")));
    p.setFont(ui::font(10.5, 700));
    QRectF tr = rtl ? QRectF(r.left() + 16, 0, r.width() - 64, height()) : QRectF(r.left() + 48, 0, r.width() - 64, height());
    p.drawText(tr, Qt::AlignCenter, text);
}

// ---- SlideStack ------------------------------------------------------------
SlideStack::SlideStack(QWidget* parent) : QWidget(parent) {}

int SlideStack::add(QWidget* w) {
    w->setParent(this);
    w->hide();
    pages.append(w);
    return static_cast<int>(pages.size()) - 1;
}

void SlideStack::resizeEvent(QResizeEvent* e) {
    for (QWidget* w : pages) {
        w->setGeometry(rect());
    }
    QWidget::resizeEvent(e);
}

void SlideStack::go(int i, int direction, bool animate) {
    if (i == index || i < 0 || i >= pages.size()) return;
    QWidget* oldW = (index >= 0 && index < pages.size()) ? pages[index] : nullptr;
    QWidget* newW = pages[i];
    index = i;
    newW->setGeometry(rect());
    newW->show();
    newW->raise();

    if (m_grp) {
        m_grp->stop();
        m_grp->deleteLater();
        m_grp = nullptr;
    }
    if (!animate || !oldW) {
        if (oldW) oldW->hide();
        return;
    }

    int dx = 70 * direction * (isRightToLeft() ? -1 : 1);
    m_grp = new QParallelAnimationGroup(this);

    auto addAnim = [&](QWidget* w, QPoint startPos, QPoint endPos, double o0, double o1) {
        auto* fx = new QGraphicsOpacityEffect(w);
        w->setGraphicsEffect(fx);
        auto* a = new QPropertyAnimation(fx, "opacity", m_grp);
        a->setStartValue(o0);
        a->setEndValue(o1);
        a->setDuration(320);
        a->setEasingCurve(QEasingCurve::OutCubic);
        auto* b = new QPropertyAnimation(w, "pos", m_grp);
        b->setStartValue(startPos);
        b->setEndValue(endPos);
        b->setDuration(320);
        b->setEasingCurve(QEasingCurve::OutCubic);
        m_grp->addAnimation(a);
        m_grp->addAnimation(b);
    };

    addAnim(newW, QPoint(dx, 0), QPoint(0, 0), 0.0, 1.0);
    addAnim(oldW, QPoint(0, 0), QPoint(-dx, 0), 1.0, 0.0);

    connect(m_grp, &QParallelAnimationGroup::finished, this, [oldW, newW]() {
        oldW->hide();
        oldW->move(0, 0);
        oldW->setGraphicsEffect(nullptr);
        newW->setGraphicsEffect(nullptr);
    });
    m_grp->start();
}

} // namespace vm::ui
