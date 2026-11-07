#include "notification.h"
#include "../i18n.h"
#include "../theme.h"
#include "art.h"
#include "icons.h"
#include "widgets.h"
#include <QBrush>
#include <QEasingCurve>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPropertyAnimation>
#include <QScreen>
#include <cmath>

namespace vm::ui {

static constexpr int W = 420;
static constexpr int H = 372;
static constexpr int H_SHORT = 262;
static constexpr int PAD = 10;

namespace {
class BellBadge : public QWidget {
public:
    BellBadge() { setFixedSize(54, 54); }
protected:
    void paintEvent(QPaintEvent*) override {
        auto& T = theme::T();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient g(0, 0, 54, 54);
        g.setColorAt(0, T.c(QStringLiteral("accent_hi")));
        g.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(QPen(QColor(255, 255, 255, 90), 1));
        p.setBrush(QBrush(g));
        p.drawRoundedRect(QRectF(0.5, 0.5, 53, 53), 18, 18);
        icons::draw(&p, QStringLiteral("bell"), QRectF(14, 14, 26, 26), T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")), 2.0);
    }
};
} // namespace

NotificationPopup::NotificationPopup(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(W, H);
    m_auto = new QTimer(this);
    m_auto->setSingleShot(true);
    connect(m_auto, &QTimer::timeout, this, &NotificationPopup::dismiss);
    root = new QVBoxLayout(this);
    root->setContentsMargins(PAD + 18, PAD + 16, PAD + 18, PAD + 18);
    root->setSpacing(10);
    setStyleSheet(QStringLiteral(R"(
        QLineEdit { background: rgba(255,255,255,0.16); color:#FFFFFF; border:1px solid rgba(255,255,255,0.34);
                    border-radius: 13px; padding: 4px 10px; selection-background-color: rgba(255,255,255,0.4); }
        QLineEdit:focus { border:1px solid rgba(255,255,255,0.85); }
    )"));
}

void NotificationPopup::clear_layout(QLayout* lay) {
    while (QLayoutItem* it = lay->takeAt(0)) {
        if (QWidget* w = it->widget()) {
            w->hide();
            w->deleteLater();
        } else if (QLayout* childLay = it->layout()) {
            clear_layout(childLay);
            childLay->deleteLater();
        }
        delete it;
    }
}

void NotificationPopup::build(const QString& title, const QString& name, const QString& sub, int snooze_default, bool with_snooze) {
    clear_layout(root);
    auto* top = new QHBoxLayout();
    top->setSpacing(12);
    top->addWidget(new BellBadge());

    auto* col = new QVBoxLayout();
    col->setSpacing(0);
    auto* a = new Label(title, 10.5, 700, QStringLiteral("#FFFFFF"));
    auto* b = new Label(name, 26.0, 800, QStringLiteral("#FFFFFF"));
    auto* c = new Label(sub, 10.5, 500, QStringLiteral("rgba(255,255,255,0.82)"));
    a->setStyleSheet(QStringLiteral("color:#F4DFA6; background:transparent;"));
    col->addWidget(a);
    col->addWidget(b);
    col->addWidget(c);
    top->addLayout(col, 1);

    auto* x = new IconButton(QStringLiteral("x"), 32, QString(), nullptr, 10, QStringLiteral("text2"), 15);
    x->fixed_color = QColor(QStringLiteral("#FFFFFF"));
    connect(x, &QAbstractButton::clicked, this, &NotificationPopup::stop);
    top->addWidget(x, 0, Qt::AlignTop);
    root->addLayout(top);
    root->addStretch(1);

    auto* row = new QHBoxLayout();
    row->setSpacing(10);
    auto* stopBtn = new Button(i18n::t(QStringLiteral("stop")), QStringLiteral("primary"), QStringLiteral("stop"), nullptr, 44);
    connect(stopBtn, &QAbstractButton::clicked, this, &NotificationPopup::stop);
    auto* opnBtn = new Button(i18n::t(QStringLiteral("open_app")), QStringLiteral("glass"), QString(), nullptr, 44);
    connect(opnBtn, &QAbstractButton::clicked, this, &NotificationPopup::do_open);
    row->addWidget(stopBtn, 1);
    row->addWidget(opnBtn, 1);
    root->addLayout(row);

    if (!with_snooze) {
        return;
    }

    auto* sn = new QHBoxLayout();
    sn->setSpacing(8);
    for (int m : {5, 10, 15}) {
        auto* btn = new Button(i18n::t(QStringLiteral("snooze_n"), {{QStringLiteral("n"), QString::number(m)}}),
                               QStringLiteral("glass"), QStringLiteral("snooze"), nullptr, 38);
        connect(btn, &QAbstractButton::clicked, this, [this, m]() { do_snooze(m); });
        sn->addWidget(btn, 1);
    }
    root->addLayout(sn);

    auto* cu = new QHBoxLayout();
    cu->setSpacing(8);
    minutes = new QLineEdit(QString::number(snooze_default));
    minutes->setValidator(new QIntValidator(1, 720, this));
    minutes->setAlignment(Qt::AlignCenter);
    minutes->setFixedSize(76, 38);
    minutes->setFont(ui::font(11.5, 700));
    minutes->setPlaceholderText(i18n::t(QStringLiteral("snooze_ph")));
    connect(minutes, &QLineEdit::returnPressed, this, &NotificationPopup::do_snooze_custom);
    auto* go = new Button(i18n::t(QStringLiteral("snooze_custom")), QStringLiteral("glass"), QStringLiteral("snooze"), nullptr, 38);
    connect(go, &QAbstractButton::clicked, this, &NotificationPopup::do_snooze_custom);
    auto* unit = new Label(i18n::t(QStringLiteral("minutes")), 10.0, 600, QStringLiteral("rgba(255,255,255,0.85)"));
    cu->addWidget(minutes);
    cu->addWidget(unit);
    cu->addWidget(go, 1);
    root->addLayout(cu);
}

void NotificationPopup::present(const QString& key_, const QString& time_text, const QString& kind, int pre_minutes, int snooze_default) {
    key = key_;
    QString title;
    QString name = i18n::t(key);
    if (kind == QStringLiteral("pre")) {
        title = (pre_minutes > 0)
                    ? i18n::t(QStringLiteral("pre_title"), {{QStringLiteral("name"), name}, {QStringLiteral("n"), QString::number(pre_minutes)}})
                    : i18n::t(QStringLiteral("next_prayer"));
    } else if (kind == QStringLiteral("snooze")) {
        title = i18n::t(QStringLiteral("snoozed"));
    } else if (kind == QStringLiteral("iqama")) {
        title = i18n::t(QStringLiteral("iqama_title"));
    } else {
        title = i18n::t(QStringLiteral("time_for"));
    }

    QString mark = i18n::is_rtl() ? QStringLiteral("\u200f") : QStringLiteral("\u200e");
    bool with_snooze = (kind != QStringLiteral("iqama"));
    setFixedSize(W, with_snooze ? H : H_SHORT);
    QString secLang = (i18n::lang() == QStringLiteral("ar")) ? QStringLiteral("en") : QStringLiteral("ar");
    build(title, name, QStringLiteral("%1%2  ·  %3").arg(mark, time_text, i18n::tl(key, secLang)), snooze_default, with_snooze);

    double dpr = devicePixelRatioF();
    int iw = W - 2 * PAD;
    int ih = height() - 2 * PAD;
    m_bg = art::render(key, iw, ih, dpr, ih * 0.78, iw * 0.5);

    QRect scr = QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->availableGeometry() : QRect(0, 0, 1920, 1080);
    QPoint endPos(scr.right() - width() - 14, scr.bottom() - height() - 14);
    QPoint startPos = endPos + QPoint(60, 0);
    move(startPos);
    setWindowOpacity(0.0);
    show();
    raise();

    if (m_grp) {
        m_grp->stop();
        m_grp->deleteLater();
    }
    m_grp = new QParallelAnimationGroup(this);
    auto* a = new QPropertyAnimation(this, "pos", m_grp);
    a->setStartValue(startPos);
    a->setEndValue(endPos);
    a->setDuration(420);
    a->setEasingCurve(QEasingCurve::OutCubic);
    auto* o = new QPropertyAnimation(this, "windowOpacity", m_grp);
    o->setStartValue(0.0);
    o->setEndValue(1.0);
    o->setDuration(320);
    m_grp->addAnimation(a);
    m_grp->addAnimation(o);
    m_grp->start();

    m_auto->start((kind == QStringLiteral("pre") || kind == QStringLiteral("iqama")) ? 60000 : 180000);
}

void NotificationPopup::dismiss() {
    m_auto->stop();
    if (m_grp) {
        m_grp->stop();
        m_grp->deleteLater();
    }
    m_grp = new QParallelAnimationGroup(this);
    auto* a = new QPropertyAnimation(this, "pos", m_grp);
    a->setStartValue(pos());
    a->setEndValue(pos() + QPoint(60, 0));
    a->setDuration(260);
    auto* o = new QPropertyAnimation(this, "windowOpacity", m_grp);
    o->setStartValue(windowOpacity());
    o->setEndValue(0.0);
    o->setDuration(260);
    m_grp->addAnimation(a);
    m_grp->addAnimation(o);
    connect(m_grp, &QParallelAnimationGroup::finished, this, &QWidget::hide);
    m_grp->start();
}

void NotificationPopup::stop() {
    emit stopped();
    dismiss();
}

void NotificationPopup::do_snooze(int m) {
    emit snoozed(key, std::max(1, m));
    dismiss();
}

void NotificationPopup::do_snooze_custom() {
    if (!minutes) return;
    bool ok = false;
    int v = minutes->text().trimmed().toInt(&ok);
    if (ok && v > 0) {
        do_snooze(v);
    }
}

void NotificationPopup::do_open() {
    emit openApp();
    dismiss();
}

void NotificationPopup::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRectF r = QRectF(rect()).adjusted(PAD, PAD, -PAD, -PAD);
    for (int i = 0; i < PAD; ++i) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, static_cast<int>(34.0 * std::pow(1.0 - static_cast<double>(i) / PAD, 1.5))));
        p.drawRoundedRect(r.adjusted(-i, -i + 4, i, i + 4), 28 + i, 28 + i);
    }
    QPainterPath path;
    path.addRoundedRect(r, 28, 28);
    p.save();
    p.setClipPath(path);
    if (!m_bg.isNull()) {
        p.drawPixmap(r, m_bg, QRectF(0, 0, m_bg.width(), m_bg.height()));
    }
    QLinearGradient g(0, r.top(), 0, r.bottom());
    g.setColorAt(0.00, QColor(4, 6, 12, 120));
    g.setColorAt(0.30, QColor(4, 6, 12, 40));
    g.setColorAt(0.52, QColor(4, 6, 12, 150));
    g.setColorAt(1.00, QColor(4, 6, 12, 225));
    p.fillRect(r, QBrush(g));
    p.restore();

    QLinearGradient hl(r.left(), 0, r.right(), 0);
    QColor c0 = T.c(QStringLiteral("accent_hi"), 0.0);
    QColor c1 = T.c(QStringLiteral("accent_hi"), 0.9);
    hl.setColorAt(0.0, c0);
    hl.setColorAt(0.5, c1);
    hl.setColorAt(1.0, c0);
    p.setPen(QPen(QColor(255, 255, 255, 60), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, 28, 28);
    p.setPen(QPen(QBrush(hl), 2));
    p.drawLine(QPointF(r.left() + 50, r.top() + 1), QPointF(r.right() - 50, r.top() + 1));
}

} // namespace vm::ui
