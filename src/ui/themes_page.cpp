#include "themes_page.h"
#include "../i18n.h"
#include "../theme.h"
#include "home.h"
#include "icons.h"
#include <QBrush>
#include <QColorDialog>
#include <QConicalGradient>
#include <QEasingCurve>
#include <QFrame>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <algorithm>

namespace vm::ui {

PaletteCard::PaletteCard(const QString& name_, bool selected, QWidget* parent)
    : QAbstractButton(parent), name(name_), sel(selected) {
    setFixedSize(W, H);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
}

void PaletteCard::enterEvent(QEnterEvent* e) {
    go(1.0);
    QAbstractButton::enterEvent(e);
}

void PaletteCard::leaveEvent(QEvent* e) {
    go(0.0);
    QAbstractButton::leaveEvent(e);
}

void PaletteCard::go(double v) {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = new QVariantAnimation(this);
    m_a->setStartValue(m_h);
    m_a->setEndValue(v);
    m_a->setDuration(180);
    m_a->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_a, &QVariantAnimation::valueChanged, this, [this](const QVariant& x) {
        m_h = x.toDouble();
        update();
    });
    m_a->start();
}

void PaletteCard::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    auto P = T.preview(name, T.mode);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(3, 3 + (1.0 - m_h) * 3.0, -3, -3);

    // outer card
    p.setPen(QPen(sel ? T.c(QStringLiteral("accent")) : T.c(QStringLiteral("border_hi")), sel ? 2.2 : 1.0));
    p.setBrush(T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(QStringLiteral("#FFFFFF")));
    p.drawRoundedRect(r, 24, 24);

    // mini window
    QRectF m(r.left() + 12, r.top() + 12, r.width() - 24, 112);
    QLinearGradient g(m.topLeft(), m.bottomRight());
    g.setColorAt(0, P.value(QStringLiteral("bg0")));
    g.setColorAt(1, P.value(QStringLiteral("bg1")));
    p.setPen(QPen(P.value(QStringLiteral("border_hi")), 1));
    p.setBrush(QBrush(g));
    p.drawRoundedRect(m, 16, 16);

    // sidebar strip + header
    p.setPen(Qt::NoPen);
    p.setBrush(P.value(QStringLiteral("bg2")));
    p.drawRoundedRect(QRectF(m.left() + 6, m.top() + 6, 14, m.height() - 12), 7, 7);

    QLinearGradient ag(m.left(), 0, m.right(), 0);
    ag.setColorAt(0, P.value(QStringLiteral("accent")));
    ag.setColorAt(1, P.value(QStringLiteral("accent2")));
    p.setBrush(QBrush(ag));
    p.drawRoundedRect(QRectF(m.left() + 9, m.top() + 10, 8, 8), 3, 3);
    p.drawRoundedRect(QRectF(m.left() + 30, m.top() + 10, m.width() * 0.34, 8), 4, 4);

    // hero strip
    QRectF hs(m.left() + 28, m.top() + 26, m.width() - 38, 26);
    QLinearGradient hg(hs.left(), 0, hs.right(), 0);
    hg.setColorAt(0, P.value(QStringLiteral("accent2")));
    hg.setColorAt(1, P.value(QStringLiteral("accent")));
    QColor hc = P.value(QStringLiteral("accent"));
    hc.setAlphaF(0.30f);
    p.setBrush(hc);
    p.drawRoundedRect(hs, 9, 9);
    p.setBrush(QBrush(hg));
    p.drawRoundedRect(QRectF(hs.left() + 8, hs.top() + 8, hs.width() * 0.35, 10), 5, 5);

    // three arches
    double aw = (m.width() - 38.0 - 12.0) / 3.0;
    for (int i = 0; i < 3; ++i) {
        QRectF ar(m.left() + 28.0 + i * (aw + 6.0), m.top() + 60.0, aw, 44.0);
        QColor fill = P.value(QStringLiteral("text"));
        fill.setAlphaF(0.10f);
        p.setPen((i == 1) ? QPen(P.value(QStringLiteral("accent")), 1.6) : Qt::NoPen);
        p.setBrush(fill);
        p.drawPath(arch_path(ar, 8));
    }

    // label + swatches
    p.setPen(T.c(QStringLiteral("text")));
    p.setFont(ui::font(11.5, 800));
    QString arLabel = QStringLiteral("لونك الخاص");
    QString enLabel = QStringLiteral("Your own color");
    if (theme::PALETTES().contains(name)) {
        const auto& pe = theme::PALETTES().value(name);
        arLabel = pe.ar;
        enLabel = pe.en;
    }
    QString label = (i18n::lang() == QStringLiteral("ar")) ? arLabel : enLabel;
    QRectF lr = i18n::is_rtl() ? QRectF(r.left() + 72, r.top() + 132, r.width() - 88, 36)
                               : QRectF(r.left() + 16, r.top() + 132, r.width() - 88, 36);
    p.drawText(lr, (i18n::is_rtl() ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter | Qt::AlignAbsolute, label);

    double sx = i18n::is_rtl() ? (r.left() + 16.0) : (r.right() - 16.0 - 44.0);
    const QColor swCols[] = {P.value(QStringLiteral("accent")), P.value(QStringLiteral("accent2"))};
    for (int i = 0; i < 2; ++i) {
        p.setPen(QPen(T.dark() ? QColor(255, 255, 255, 80) : QColor(0, 0, 0, 40), 1));
        p.setBrush(swCols[i]);
        p.drawEllipse(QRectF(sx + i * 22.0, r.top() + 141.0, 18.0, 18.0));
    }

    if (name == QStringLiteral("custom")) {
        QConicalGradient cg(m.center(), 0);
        const QString rainbow[] = {
            QStringLiteral("#FF5C5C"), QStringLiteral("#FFC24A"), QStringLiteral("#7BE06A"),
            QStringLiteral("#4AC8FF"), QStringLiteral("#8A7BFF"), QStringLiteral("#FF5CC8"),
            QStringLiteral("#FF5C5C")
        };
        for (int i = 0; i < 7; ++i) {
            cg.setColorAt(i / 6.0, QColor(rainbow[i]));
        }
        p.setPen(QPen(QColor(255, 255, 255, 220), 2.4));
        p.setBrush(QBrush(cg));
        p.drawEllipse(m.center(), 17, 17);
        p.setPen(Qt::NoPen);
        p.setBrush(P.value(QStringLiteral("bg0")));
        p.drawEllipse(m.center(), 6, 6);
    }

    if (sel) {
        QRectF cr(r.right() - 36, r.top() + 18, 24, 24);
        QLinearGradient g2(cr.topLeft(), cr.bottomRight());
        g2.setColorAt(0, T.c(QStringLiteral("accent_hi")));
        g2.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(g2));
        p.drawEllipse(cr);
        icons::draw(&p, QStringLiteral("check"), cr.adjusted(5, 5, -5, -5), T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")), 2.6);
    }
}

ThemesPage::ThemesPage(Settings* settings, QWidget* parent)
    : QWidget(parent), s(settings) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(30, 14, 30, 0);
    outer->setSpacing(14);
    outer->addWidget(new Label(i18n::t(QStringLiteral("themes")), 27.0, 800, QStringLiteral("text")));
    outer->addWidget(new Label(i18n::t(QStringLiteral("theme_hint")), 11.5, 500, QStringLiteral("text2")));
    outer->addSpacing(4);
    outer->addWidget(new Label(i18n::t(QStringLiteral("theme_mode")), 12.0, 700, QStringLiteral("accent")));

    mode = new Segmented(
        {{QStringLiteral("dark"), i18n::t(QStringLiteral("dark"))},
         {QStringLiteral("light"), i18n::t(QStringLiteral("light"))},
         {QStringLiteral("auto"), i18n::t(QStringLiteral("auto"))}},
        s->theme,
        nullptr,
        {{QStringLiteral("dark"), QStringLiteral("moon")},
         {QStringLiteral("light"), QStringLiteral("sun")},
         {QStringLiteral("auto"), QStringLiteral("compass")}},
        42,
        150
    );
    connect(mode, &Segmented::changed, this, &ThemesPage::modeChanged);
    outer->addWidget(mode, 0, Qt::AlignLeading);
    outer->addSpacing(6);
    outer->addWidget(new Label(i18n::t(QStringLiteral("palette")), 12.0, 700, QStringLiteral("accent")));

    scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    host = new QWidget();
    grid = new QGridLayout(host);
    grid->setContentsMargins(0, 6, 8, 24);
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(16);
    scroll->setWidget(host);
    outer->addWidget(scroll, 1);

    QStringList names;
    names.append(QStringLiteral("custom"));
    names.append(theme::PALETTE_ORDER());
    for (const QString& n : names) {
        auto* c = new PaletteCard(n, n == s->palette);
        connect(c, &QAbstractButton::clicked, this, [this, n]() { pick(n); });
        cards.append(c);
    }
    flow();
}

void ThemesPage::pick(const QString& name) {
    if (name == QStringLiteral("custom")) {
        QColor col = QColorDialog::getColor(QColor(s->custom_accent), window(), i18n::t(QStringLiteral("custom_color")));
        if (col.isValid()) {
            emit customChosen(col.name().toUpper());
        }
        return;
    }
    if (name == s->palette) {
        return;
    }
    for (PaletteCard* c : cards) {
        c->sel = (c->name == name);
        c->update();
    }
    emit paletteChanged(name);
}

void ThemesPage::resizeEvent(QResizeEvent* e) {
    flow();
    QWidget::resizeEvent(e);
}

void ThemesPage::flow() {
    int avail = std::max(300, width() - 60 - contentsMargins().left() - contentsMargins().right());
    int cols = std::max(1, avail / (PaletteCard::W + 16));
    if (cols == m_cols) {
        return;
    }
    m_cols = cols;
    while (QLayoutItem* it = grid->takeAt(0)) {
        delete it;
    }
    for (int i = 0; i < cards.size(); ++i) {
        grid->addWidget(cards[i], i / cols, i % cols, Qt::AlignLeft | Qt::AlignTop);
    }
    for (int c = 0; c <= cols; ++c) {
        grid->setColumnStretch(c, (c == cols) ? 1 : 0);
    }
    int rows = (static_cast<int>(cards.size()) + cols - 1) / cols;
    for (int rr = 0; rr <= rows; ++rr) {
        grid->setRowStretch(rr, (rr == rows) ? 1 : 0);
    }
}

} // namespace vm::ui
