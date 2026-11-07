#include "about_dialog.h"
#include "../app_info.h"
#include "../i18n.h"
#include "../theme.h"
#include "art.h"
#include "icons.h"
#include "widgets.h"
#include <QBrush>
#include <QClipboard>
#include <QDesktopServices>
#include <QEasingCurve>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QScrollArea>
#include <QUrl>
#include <QVBoxLayout>
#include <cmath>

namespace vm::ui {

static constexpr int HEADER = 124;

namespace {

class Divider : public QWidget {
public:
    Divider() { setFixedHeight(18); }
protected:
    void paintEvent(QPaintEvent*) override {
        auto& T = theme::T();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        double cy = height() / 2.0;
        double w = width();
        const std::pair<double, double> segs[] = {{0.0, w / 2.0 - 16.0}, {w / 2.0 + 16.0, w}};
        for (const auto& [x0, x1] : segs) {
            QLinearGradient g(x0, 0, x1, 0);
            QColor a = T.c(QStringLiteral("accent"), 0.0);
            QColor b = T.c(QStringLiteral("accent"), 0.55);
            g.setColorAt(0, (x0 == 0.0) ? a : b);
            g.setColorAt(1, (x0 == 0.0) ? b : a);
            p.setPen(QPen(QBrush(g), 1.2));
            p.drawLine(QPointF(x0, cy), QPointF(x1, cy));
        }
        p.setPen(Qt::NoPen);
        p.setBrush(T.c(QStringLiteral("accent")));
        QPainterPath path;
        double c = w / 2.0;
        path.moveTo(c, cy - 5.0);
        path.lineTo(c + 5.0, cy);
        path.lineTo(c, cy + 5.0);
        path.lineTo(c - 5.0, cy);
        path.closeSubpath();
        p.drawPath(path);
    }
};

class ContactRow : public QFrame {
public:
    QString icon_name;
    QString text;

    ContactRow(const QString& icon_, const QString& text_, std::function<void()> copy_cb, std::function<void()> open_cb = nullptr, const QString& tip_open = QString())
        : icon_name(icon_), text(text_) {
        setFixedHeight(50);
        auto* lay = new QHBoxLayout(this);
        lay->setContentsMargins(66, 0, 10, 0);
        lay->setSpacing(6);
        lay->addStretch(1);
        if (open_cb) {
            auto* b = new IconButton(QStringLiteral("mail"), 38, tip_open, nullptr, 12);
            connect(b, &QAbstractButton::clicked, this, [open_cb]() { open_cb(); });
            lay->addWidget(b);
        }
        auto* c = new IconButton(QStringLiteral("copy"), 38, i18n::t(QStringLiteral("copy")), nullptr, 12);
        connect(c, &QAbstractButton::clicked, this, [copy_cb]() { copy_cb(); });
        lay->addWidget(c);
        setLayoutDirection(Qt::LeftToRight);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        auto& T = theme::T();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        p.setPen(QPen(T.c(QStringLiteral("border")), 1));
        p.setBrush(T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(27, 36, 32, 10));
        p.drawRoundedRect(r, 16, 16);

        QRectF ic(16, r.center().y() - 17, 34, 34);
        QLinearGradient g(ic.topLeft(), ic.bottomRight());
        g.setColorAt(0, T.c(QStringLiteral("accent_hi")));
        g.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(g));
        p.drawRoundedRect(ic, 11, 11);
        icons::draw(&p, icon_name, ic.adjusted(8, 8, -8, -8), T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")), 2.0);
        p.setPen(T.c(QStringLiteral("text")));
        p.setFont(ui::font(11.5, 600));
        p.setLayoutDirection(Qt::LeftToRight);
        p.drawText(QRectF(60, 0, width() - 160, height()), Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignAbsolute, text);
    }
};

class AboutCard : public QWidget {
public:
    QPixmap m_scene;
    AboutCard(const QString& scene_key, int w, int h, double dpr, QWidget* parent = nullptr)
        : QWidget(parent) {
        setFixedSize(w, h);
        m_scene = art::render(scene_key, w, HEADER, dpr, HEADER * 1.05, w * 0.5);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        auto& T = theme::T();
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        int pad = 14;
        QRectF r = QRectF(rect()).adjusted(pad, pad, -pad, -pad);
        for (int i = 0; i < pad; ++i) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, static_cast<int>((T.dark() ? 46 : 30) * std::pow(1.0 - static_cast<double>(i) / pad, 1.6))));
            p.drawRoundedRect(r.adjusted(-i, -i + 6, i, i + 6), 30 + i, 30 + i);
        }
        QPainterPath path;
        path.addRoundedRect(r, 30, 30);
        p.setPen(QPen(T.c(QStringLiteral("border_hi")), 1));
        QColor base = T.dark() ? T.c(QStringLiteral("bg1")).lighter(118) : QColor(QStringLiteral("#FFFFFF"));
        p.setBrush(base);
        p.drawPath(path);

        p.save();
        p.setClipPath(path);
        QRectF hr(r.left(), r.top(), r.width(), HEADER);
        p.drawPixmap(hr, m_scene, QRectF(0, 0, m_scene.width(), m_scene.height()));
        QLinearGradient fade(0, hr.bottom() - 52, 0, hr.bottom());
        QColor c0(base);
        c0.setAlpha(0);
        fade.setColorAt(0, c0);
        fade.setColorAt(1, base);
        p.fillRect(QRectF(hr.left(), hr.bottom() - 52, hr.width(), 53), QBrush(fade));
        p.restore();

        double bx = r.center().x() - 36.0;
        double by = hr.bottom() - 40.0;
        p.setPen(QPen(base, 5));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(bx, by, 72, 72), 24, 24);
        QPixmap logo = icons::logo_pixmap(72);
        p.drawPixmap(QRectF(bx, by, 72, 72), logo, QRectF(0, 0, logo.width(), logo.height()));
    }
};

} // namespace

AboutDialog::AboutDialog(QWidget* parent, const QString& scene_key_, std::function<void(const QString&)> toast_cb_)
    : QWidget(parent), scene_key(scene_key_), toast_cb(std::move(toast_cb_)) {
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground, false);
    m_fx = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_fx);
}

void AboutDialog::build() {
    int pw = width(), ph = height();
    int cw = std::min(600, std::max(380, pw - 40));
    int ch = std::min(800, std::max(440, ph - 30));
    m_card = new AboutCard(scene_key, cw, ch, devicePixelRatioF(), this);
    auto* lay = new QVBoxLayout(m_card);
    lay->setContentsMargins(14, 14, 14, 14);
    lay->setSpacing(0);
    lay->addSpacing(HEADER - 14 + 4);

    auto* sc = new QScrollArea();
    sc->setWidgetResizable(true);
    sc->setFrameShape(QFrame::NoFrame);
    sc->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* body = new QWidget();
    auto* col = new QVBoxLayout(body);
    col->setContentsMargins(34, 40, 34, 16);
    col->setSpacing(9);

    QString mark = i18n::is_rtl() ? QStringLiteral("\u200f") : QStringLiteral("\u200e");
    col->addWidget(new Label(APP_NAME, 25.0, 800, QStringLiteral("text"), nullptr, Qt::AlignCenter));
    col->addWidget(new Label(QStringLiteral("%1   ·   %2 %3").arg(i18n::t(QStringLiteral("about_kicker")), i18n::t(QStringLiteral("version")), APP_VERSION),
                             9.5, 700, QStringLiteral("accent"), nullptr, Qt::AlignCenter));
    col->addSpacing(2);

    auto* lb = new Label(mark + i18n::t(QStringLiteral("about_body")), 11.5, 500, QStringLiteral("text"), nullptr, Qt::AlignCenter);
    lb->setWordWrap(true);
    col->addWidget(lb);

    col->addWidget(new Divider());

    auto* d1 = new Label(mark + i18n::t(QStringLiteral("about_dua")), 11.5, 600, QStringLiteral("accent"), nullptr, Qt::AlignCenter);
    d1->setWordWrap(true);
    col->addWidget(d1);
    col->addWidget(new Label(i18n::t(QStringLiteral("about_amen")), 12.0, 800, QStringLiteral("accent"), nullptr, Qt::AlignCenter));

    col->addWidget(new Divider());

    col->addWidget(new Label(i18n::t(QStringLiteral("developer")), 9.5, 700, QStringLiteral("text2"), nullptr, Qt::AlignCenter));
    col->addWidget(new Label(DEV_NAME, 18.0, 800, QStringLiteral("text"), nullptr, Qt::AlignCenter));
    col->addSpacing(2);

    col->addWidget(new ContactRow(
        QStringLiteral("mail"),
        DEV_EMAIL,
        [this]() { copy_text(DEV_EMAIL); },
        []() { QDesktopServices::openUrl(QUrl(QStringLiteral("mailto:") + DEV_EMAIL)); },
        i18n::t(QStringLiteral("send_mail"))
    ));
    col->addWidget(new ContactRow(
        QStringLiteral("phone"),
        DEV_PHONE,
        [this]() { copy_text(DEV_PHONE); }
    ));

    col->addSpacing(2);
    auto* d3 = new Label(mark + i18n::t(QStringLiteral("about_dua2")), 10.5, 600, QStringLiteral("text2"), nullptr, Qt::AlignCenter);
    d3->setWordWrap(true);
    col->addWidget(d3);
    col->addSpacing(4);
    col->addWidget(new Label(mark + i18n::t(QStringLiteral("made_with")), 9.5, 500, QStringLiteral("text3"), nullptr, Qt::AlignCenter));
    auto* heart = new Label(QStringLiteral("%1♥  %2").arg(mark, i18n::t(QStringLiteral("made_care"))), 9.5, 600, QStringLiteral("text2"), nullptr, Qt::AlignCenter);
    heart->setWordWrap(true);
    col->addWidget(heart);

    sc->setWidget(body);
    lay->addWidget(sc, 1);

    auto* x = new IconButton(QStringLiteral("x"), 36, QString(), m_card, 12, QStringLiteral("text2"), 16);
    x->fixed_color = QColor(QStringLiteral("#FFFFFF"));
    x->move(cw - 14 - 14 - 36, 14 + 12);
    connect(x, &QAbstractButton::clicked, this, &AboutDialog::close_animated);
    x->raise();

    m_card->move((pw - cw) / 2, (ph - ch) / 2);
}

void AboutDialog::open() {
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
    }
    build();
    show();
    raise();
    setFocus();

    m_a = new QPropertyAnimation(m_fx, "opacity", this);
    m_a->setStartValue(0.0);
    m_a->setEndValue(1.0);
    m_a->setDuration(220);
    m_a->start();

    QPoint endPos = m_card->pos();
    m_card->move(endPos + QPoint(0, 26));
    m_b = new QPropertyAnimation(m_card, "pos", this);
    m_b->setStartValue(endPos + QPoint(0, 26));
    m_b->setEndValue(endPos);
    m_b->setDuration(320);
    m_b->setEasingCurve(QEasingCurve::OutCubic);
    m_b->start();
}

void AboutDialog::close_animated() {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = new QPropertyAnimation(m_fx, "opacity", this);
    m_a->setStartValue(1.0);
    m_a->setEndValue(0.0);
    m_a->setDuration(170);
    connect(m_a, &QPropertyAnimation::finished, this, &AboutDialog::on_done);
    m_a->start();
}

void AboutDialog::on_done() {
    hide();
    emit closed();
    deleteLater();
}

void AboutDialog::copy_text(const QString& text) {
    if (auto* cb = QGuiApplication::clipboard()) {
        cb->setText(text);
    }
    if (toast_cb) {
        toast_cb(i18n::t(QStringLiteral("copied")));
    }
}

void AboutDialog::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) {
        close_animated();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void AboutDialog::mousePressEvent(QMouseEvent* e) {
    if (m_card && !m_card->geometry().adjusted(14, 14, -14, -14).contains(e->position().toPoint())) {
        close_animated();
    }
}

void AboutDialog::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int m = (parentWidget() && parentWidget()->layout()) ? parentWidget()->layout()->contentsMargins().left() : 0;
    QRectF r = QRectF(rect()).adjusted(m, m, -m, -m);
    p.setPen(Qt::NoPen);
    p.setBrush(T.dark() ? QColor(4, 8, 6, 175) : QColor(30, 22, 8, 120));
    p.drawRoundedRect(r, m ? 20 : 0, m ? 20 : 0);
}

} // namespace vm::ui
