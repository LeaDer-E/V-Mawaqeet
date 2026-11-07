#include "onboarding.h"
#include "../i18n.h"
#include "../locations.h"
#include "../methods.h"
#include "../theme.h"
#include "icons.h"
#include <QBrush>
#include <QEasingCurve>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <algorithm>

namespace vm::ui {

static int dot_of(int page) {
    static const int map[] = {0, 1, 1, 1, 2, 3, 4};
    if (page < 0 || page > 6) return 0;
    return map[page];
}

// ---- OptionCard ------------------------------------------------------------
OptionCard::OptionCard(const QString& icon, const QString& title_, const QString& sub_, bool selected, const QString& glyph, QWidget* parent)
    : QAbstractButton(parent), icon_name(icon), title(title_), sub(sub_), sel(selected), glyph_text(glyph) {
    setFixedSize(250, 210);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
}

void OptionCard::enterEvent(QEnterEvent* e) {
    go(1.0);
    QAbstractButton::enterEvent(e);
}

void OptionCard::leaveEvent(QEvent* e) {
    go(0.0);
    QAbstractButton::leaveEvent(e);
}

void OptionCard::go(double v) {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = new QVariantAnimation(this);
    m_a->setStartValue(m_h);
    m_a->setEndValue(v);
    m_a->setDuration(200);
    connect(m_a, &QVariantAnimation::valueChanged, this, [this](const QVariant& x) {
        m_h = x.toDouble();
        update();
    });
    m_a->start();
}

void OptionCard::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(2, 2 + (1.0 - m_h) * 4.0, -2, -2);
    p.setPen(QPen((sel || m_h > 0.6) ? T.c(QStringLiteral("accent")) : T.c(QStringLiteral("border_hi")), sel ? 2 : 1));
    p.setBrush(T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(QStringLiteral("#FFFFFF")));
    p.drawRoundedRect(r, 26, 26);

    QLinearGradient g(0, 0, 64, 64);
    g.setColorAt(0, T.c(QStringLiteral("accent_hi")));
    g.setColorAt(1, T.c(QStringLiteral("accent2")));
    QRectF ir(r.center().x() - 32, r.top() + 28, 64, 64);
    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(g));
    p.drawRoundedRect(ir, 22, 22);

    if (!glyph_text.isEmpty()) {
        p.setPen(T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")));
        p.setFont(ui::font(22.0, 800));
        p.drawText(ir, Qt::AlignCenter, glyph_text);
    } else {
        icons::draw(&p, icon_name, ir.adjusted(16, 16, -16, -16), T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")), 2.1);
    }
    p.setPen(T.c(QStringLiteral("text")));
    p.setFont(ui::font(15.0, 800));
    p.drawText(QRectF(r.left(), r.top() + 108, r.width(), 32), Qt::AlignCenter, title);
    p.setPen(T.c(QStringLiteral("text2")));
    p.setFont(ui::font(10.0, 500));
    p.drawText(QRectF(r.left() + 14, r.top() + 142, r.width() - 28, 44), Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, sub);
}

// ---- ThemeCard -------------------------------------------------------------
ThemeCard::ThemeCard(const QString& mode_, const QString& title_, bool selected, QWidget* parent)
    : QAbstractButton(parent), mode(mode_), title(title_), sel(selected) {
    setFixedSize(184, 190);
    setCursor(Qt::PointingHandCursor);
}

void ThemeCard::draw_mini(QPainter* p, const QRectF& r, bool dark) {
    QColor bg = dark ? QColor(QStringLiteral("#0C1411")) : QColor(QStringLiteral("#FAF7F0"));
    QColor fg = dark ? QColor(QStringLiteral("#E6B85C")) : QColor(QStringLiteral("#B8892B"));
    QColor card = dark ? QColor(255, 255, 255, 20) : QColor(QStringLiteral("#FFFFFF"));
    p->setPen(Qt::NoPen);
    p->setBrush(bg);
    p->drawRoundedRect(r, 12, 12);
    p->setBrush(fg);
    p->drawRoundedRect(QRectF(r.left() + 10, r.top() + 10, r.width() * 0.45, 8), 4, 4);
    p->setBrush(card);
    p->drawRoundedRect(QRectF(r.left() + 10, r.top() + 26, r.width() - 20, 28), 8, 8);
    for (int i = 0; i < 3; ++i) {
        p->setBrush(card);
        p->drawRoundedRect(QRectF(r.left() + 10 + i * (r.width() - 20) / 3.0, r.top() + 62, (r.width() - 20) / 3.0 - 5, r.height() - 72), 8, 8);
    }
}

void ThemeCard::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(2, 2, -2, -2);
    p.setPen(QPen(sel ? T.c(QStringLiteral("accent")) : T.c(QStringLiteral("border_hi")), sel ? 2 : 1));
    p.setBrush(T.dark() ? T.c(QStringLiteral("surface_hi")) : QColor(QStringLiteral("#FFFFFF")));
    p.drawRoundedRect(r, 22, 22);

    QRectF pr(r.left() + 14, r.top() + 14, r.width() - 28, 100);
    if (mode == QStringLiteral("auto")) {
        p.save();
        p.setClipRect(QRectF(pr.left(), pr.top(), pr.width() / 2.0, pr.height()));
        draw_mini(&p, pr, true);
        p.restore();
        p.save();
        p.setClipRect(QRectF(pr.center().x(), pr.top(), pr.width() / 2.0, pr.height()));
        draw_mini(&p, pr, false);
        p.restore();
    } else {
        draw_mini(&p, pr, mode == QStringLiteral("dark"));
    }
    p.setPen(T.c(QStringLiteral("text")));
    p.setFont(ui::font(13.0, 800));
    p.drawText(QRectF(r.left(), r.top() + 128, r.width(), 40), Qt::AlignCenter, title);
}

// ---- Spinner ---------------------------------------------------------------
Spinner::Spinner(QWidget* parent) : QWidget(parent) {
    setFixedSize(72, 72);
    auto* t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [this]() {
        a = (a + 8) % 360;
        update();
    });
    t->start(16);
}

void Spinner::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(T.c(QStringLiteral("border_hi")), 5));
    p.drawEllipse(QRectF(8, 8, 56, 56));
    QPen pen(T.c(QStringLiteral("accent")), 5);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.drawArc(QRectF(8, 8, 56, 56), -a * 16, 100 * 16);
}

// ---- SuccessMark -----------------------------------------------------------
SuccessMark::SuccessMark(QWidget* parent) : QWidget(parent) {
    setFixedSize(130, 130);
    m_an = new QVariantAnimation(this);
    m_an->setStartValue(0.0);
    m_an->setEndValue(1.0);
    m_an->setDuration(700);
    m_an->setEasingCurve(QEasingCurve::OutBack);
    connect(m_an, &QVariantAnimation::valueChanged, this, [this](const QVariant& x) {
        v = x.toDouble();
        update();
    });
}

void SuccessMark::play() {
    m_an->start();
}

void SuccessMark::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    double s = std::max(0.0, v);
    QPointF c(65, 65);
    p.translate(c);
    p.scale(s, s);
    p.translate(-c);

    QLinearGradient g(0, 0, 130, 130);
    g.setColorAt(0, T.c(QStringLiteral("accent_hi")));
    g.setColorAt(1, T.c(QStringLiteral("accent2")));
    const int alphas[] = {30, 18, 8};
    for (int i = 0; i < 3; ++i) {
        p.setPen(Qt::NoPen);
        p.setBrush(T.c(QStringLiteral("accent"), alphas[i] / 100.0));
        p.drawEllipse(c, 52 + i * 6, 52 + i * 6);
    }
    p.setBrush(QBrush(g));
    p.drawEllipse(c, 50, 50);
    icons::draw(&p, QStringLiteral("check"), QRectF(35, 35, 60, 60), T.dark() ? T.c(QStringLiteral("on_accent")) : QColor(QStringLiteral("#FFFFFF")), 2.6);
}

// ---- Dots ------------------------------------------------------------------
Dots::Dots(QWidget* parent) : QWidget(parent) {
    setFixedSize(150, 20);
}

void Dots::set(int idx) {
    i = idx;
    update();
}

void Dots::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    double x = 8;
    for (int k = 0; k < 5; ++k) {
        bool on = (k == i);
        double w = on ? 28.0 : 8.0;
        p.setPen(Qt::NoPen);
        p.setBrush((k <= i) ? T.c(QStringLiteral("accent")) : T.c(QStringLiteral("border_hi")));
        p.drawRoundedRect(QRectF(x, 6, w, 8), 4, 4);
        x += w + 8.0;
    }
}

// ---- Onboarding ------------------------------------------------------------
Onboarding::Onboarding(AppState* st, Locator* loc, QWidget* parent)
    : QWidget(parent), state(st), locator(loc), s(&st->s) {
    dots = new Dots();
    stack = new SlideStack();
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 10, 30, 30);
    lay->addWidget(dots, 0, Qt::AlignHCenter);
    lay->addWidget(stack, 1);
    build(0, false);
}

QWidget* Onboarding::make_page(
    const QString& title,
    const QString& sub,
    QWidget* content,
    std::optional<int> back,
    const QString& nxt_label,
    std::function<void()> nxt_cb
) {
    auto* pg = new QWidget();
    auto* outer = new QVBoxLayout(pg);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addStretch(1);

    auto* col = new QVBoxLayout();
    col->setSpacing(14);
    col->setAlignment(Qt::AlignHCenter);
    auto* a = new Label(title, 30.0, 800, QStringLiteral("text"), nullptr, Qt::AlignCenter);
    auto* b = new Label(sub, 12.0, 500, QStringLiteral("text2"), nullptr, Qt::AlignCenter);
    b->setWordWrap(true);
    col->addWidget(a);
    col->addWidget(b);
    col->addSpacing(14);
    if (content) {
        col->addWidget(content, 0, Qt::AlignHCenter);
    }
    if (back.has_value() || !nxt_label.isEmpty()) {
        col->addSpacing(18);
        auto* row = new QHBoxLayout();
        row->setSpacing(12);
        row->addStretch(1);
        if (back.has_value()) {
            int backIdx = *back;
            auto* bb = new Button(i18n::t(QStringLiteral("back")), QStringLiteral("ghost"), QString(), nullptr, 46);
            bb->setFixedWidth(120);
            connect(bb, &QAbstractButton::clicked, this, [this, backIdx]() { go(backIdx, -1); });
            row->addWidget(bb);
        }
        if (!nxt_label.isEmpty() && nxt_cb) {
            auto* nb = new Button(nxt_label, QStringLiteral("primary"), QString(), nullptr, 46);
            nb->setFixedWidth(200);
            connect(nb, &QAbstractButton::clicked, this, [nxt_cb]() { nxt_cb(); });
            row->addWidget(nb);
        }
        row->addStretch(1);
        col->addLayout(row);
    }

    auto* w = new QWidget();
    w->setFixedWidth(620);
    w->setLayout(col);
    auto* h = new QHBoxLayout();
    h->addStretch(1);
    h->addWidget(w);
    h->addStretch(1);
    outer->addLayout(h);
    outer->addStretch(2);
    return pg;
}

void Onboarding::go(int i, int direction, bool animate) {
    dots->set(dot_of(i));
    stack->go(i, direction, animate);
}

QWidget* Onboarding::make_cards(const QVector<QWidget*>& cards) {
    auto* w = new QWidget();
    auto* l = new QHBoxLayout(w);
    l->setSpacing(18);
    l->setContentsMargins(0, 0, 0, 0);
    for (QWidget* c : cards) {
        l->addWidget(c);
    }
    return w;
}

void Onboarding::build(int start, bool animate) {
    for (QWidget* pg : stack->pages) {
        pg->deleteLater();
    }
    stack->pages.clear();
    stack->index = -1;

    // 0 language
    auto* ar = new OptionCard(QString(), QStringLiteral("العربية"), QStringLiteral("واجهة كاملة من اليمين لليسار"), i18n::lang() == QStringLiteral("ar"), QStringLiteral("ع"));
    auto* en = new OptionCard(QString(), QStringLiteral("English"), QStringLiteral("Full left-to-right interface"), i18n::lang() == QStringLiteral("en"), QStringLiteral("En"));
    connect(ar, &QAbstractButton::clicked, this, [this]() { set_lang(QStringLiteral("ar")); });
    connect(en, &QAbstractButton::clicked, this, [this]() { set_lang(QStringLiteral("en")); });
    stack->add(make_page(i18n::t(QStringLiteral("ob_welcome")), QStringLiteral("اختر لغتك · Choose your language"), make_cards({ar, en})));

    // 1 mode
    auto* a = new OptionCard(QStringLiteral("locate"), i18n::t(QStringLiteral("ob_auto")), i18n::t(QStringLiteral("ob_auto_sub")));
    auto* m = new OptionCard(QStringLiteral("globe"), i18n::t(QStringLiteral("ob_manual")), i18n::t(QStringLiteral("ob_manual_sub")));
    connect(a, &QAbstractButton::clicked, this, &Onboarding::start_auto);
    connect(m, &QAbstractButton::clicked, this, [this]() { to_manual(false); });
    stack->add(make_page(i18n::t(QStringLiteral("ob_how_loc")), QString(), make_cards({a, m}), 0));

    // 2 auto
    spin = new Spinner();
    found = new Label(QString(), 20.0, 800, QStringLiteral("text"), nullptr, Qt::AlignCenter);
    found_sub = new Label(i18n::t(QStringLiteral("detecting")), 12.0, 500, QStringLiteral("text2"), nullptr, Qt::AlignCenter);
    auto* box = new QWidget();
    auto* bl = new QVBoxLayout(box);
    bl->setAlignment(Qt::AlignHCenter);
    bl->addWidget(spin, 0, Qt::AlignHCenter);
    bl->addWidget(found);
    bl->addWidget(found_sub);

    auto* row = new QHBoxLayout();
    yes = new Button(i18n::t(QStringLiteral("ob_yes")), QStringLiteral("primary"), QStringLiteral("check"), nullptr, 46);
    edit = new Button(i18n::t(QStringLiteral("ob_edit")), QStringLiteral("ghost"), QString(), nullptr, 46);
    connect(yes, &QAbstractButton::clicked, this, &Onboarding::accept_found);
    connect(edit, &QAbstractButton::clicked, this, [this]() { to_manual(true); });
    row->addWidget(edit);
    row->addWidget(yes);
    btnrow = new QWidget();
    btnrow->setLayout(row);
    bl->addWidget(btnrow);
    btnrow->hide();
    stack->add(make_page(i18n::t(QStringLiteral("ob_found")), QString(), box, 1));

    // 3 manual
    cb_country = new SearchCombo(i18n::t(QStringLiteral("search")), i18n::t(QStringLiteral("no_results")));
    cb_region = new SearchCombo(i18n::t(QStringLiteral("search")), i18n::t(QStringLiteral("no_results")));
    QString lg = i18n::lang();
    QVector<ComboItem> cItems;
    for (const auto& c : locations::countries()) {
        cItems.append(ComboItem{c.name(lg), c.code, c.code, QStringLiteral("%1 %2 %3").arg(c.en, c.ar, c.code).toLower()});
    }
    cb_country->set_items(cItems);
    cb_country->set_value(s->country_code);
    fill_regions();
    connect(cb_country, &SearchCombo::changed, this, [this](const QVariant& cVar) {
        s->country_code = cVar.toString();
        s->region_index = 0;
        fill_regions();
    });
    connect(cb_region, &SearchCombo::changed, this, [this](const QVariant& iVar) {
        s->region_index = iVar.toInt();
    });

    auto* man = new QWidget();
    man->setFixedWidth(460);
    auto* ml = new QVBoxLayout(man);
    ml->setSpacing(14);
    ml->setContentsMargins(0, 0, 0, 0);
    ml->addWidget(new Label(i18n::t(QStringLiteral("country")), 10.5, 600, QStringLiteral("text2")));
    ml->addWidget(cb_country);
    ml->addWidget(new Label(i18n::t(QStringLiteral("region")), 10.5, 600, QStringLiteral("text2")));
    ml->addWidget(cb_region);
    stack->add(make_page(i18n::t(QStringLiteral("ob_pick_loc")), QString(), man, 1, i18n::t(QStringLiteral("next")), [this]() { after_location(); }));

    // 4 method
    cb_method = new SearchCombo(i18n::t(QStringLiteral("search")), i18n::t(QStringLiteral("no_results")));
    fill_methods();
    auto* mw = new QWidget();
    mw->setFixedWidth(500);
    auto* mv = new QVBoxLayout(mw);
    mv->setContentsMargins(0, 0, 0, 0);
    mv->addWidget(cb_method);
    connect(cb_method, &SearchCombo::changed, this, [this](const QVariant& kVar) {
        QString k = kVar.toString();
        s->method = (k == methods::default_for(s->country_code)) ? QStringLiteral("auto") : k;
    });
    stack->add(make_page(i18n::t(QStringLiteral("ob_method")), i18n::t(QStringLiteral("ob_method_sub")), mw, 3, i18n::t(QStringLiteral("next")), [this]() { go(5); }));

    // 5 theme
    m_theme_cards.clear();
    QVector<QWidget*> tWidgets;
    for (const QString& k : {QStringLiteral("dark"), QStringLiteral("light"), QStringLiteral("auto")}) {
        auto* tc = new ThemeCard(k, i18n::t(k), s->theme == k);
        connect(tc, &QAbstractButton::clicked, this, [this, k]() { set_theme(k); });
        m_theme_cards.append(tc);
        tWidgets.append(tc);
    }
    stack->add(make_page(i18n::t(QStringLiteral("ob_theme")), QString(), make_cards(tWidgets), 4, i18n::t(QStringLiteral("next")), [this]() { to_done(); }));

    // 6 done
    mark = new SuccessMark();
    stack->add(make_page(i18n::t(QStringLiteral("ob_done")), i18n::t(QStringLiteral("ob_done_sub")), mark, std::nullopt, i18n::t(QStringLiteral("start")), [this]() { do_finish(); }));

    stack->resize(stack->size());
    if (start == 0) {
        dots->set(0);
        stack->go(0, 1, false);
    } else {
        stack->go(0, 1, false);
        QTimer::singleShot(60, this, [this, start, animate]() { go(start, 1, animate); });
    }
}

void Onboarding::fill_regions() {
    QString lg = i18n::lang();
    auto c = locations::by_code(s->country_code);
    QVector<ComboItem> items;
    for (int i = 0; i < c.regions.size(); ++i) {
        const auto& r = c.regions[i];
        items.append(ComboItem{locations::region_name(r, lg), QString(), i, QStringLiteral("%1 %2").arg(r.en, r.ar).toLower()});
    }
    cb_region->set_items(items);
    cb_region->set_value(s->region_index);
}

void Onboarding::fill_methods() {
    QString lg = i18n::lang();
    QString d = methods::default_for(s->country_code);
    const auto& allM = methods::METHODS();
    QVector<ComboItem> items;
    for (const QString& k : methods::ORDER()) {
        const auto& m = allM.value(k);
        QString label = ((lg == QStringLiteral("ar")) ? m.ar : m.en) + ((k == d) ? QStringLiteral(" ★") : QString());
        items.append(ComboItem{label, QString(), k, QStringLiteral("%1 %2").arg(m.en, m.ar).toLower()});
    }
    cb_method->set_items(items);
    cb_method->set_value((s->method == QStringLiteral("auto")) ? d : s->method);
}

void Onboarding::set_lang(const QString& code) {
    emit languageChanged(code);
}

void Onboarding::set_theme(const QString& k) {
    s->theme = k;
    for (ThemeCard* c : m_theme_cards) {
        c->sel = (c->mode == k);
        c->update();
    }
    emit themeChanged(k);
}

void Onboarding::start_auto() {
    mode = QStringLiteral("auto");
    spin->show();
    found->setText(QString());
    found_sub->setText(i18n::t(QStringLiteral("detecting")));
    btnrow->hide();
    go(2);
    locator->detect([this](std::optional<DetectedLocation> res) { on_detected(res); });
}

void Onboarding::on_detected(std::optional<DetectedLocation> res) {
    if (!res.has_value()) {
        found->setText(QString());
        found_sub->setText(i18n::t(QStringLiteral("ob_failed")));
        spin->hide();
        QTimer::singleShot(1600, this, [this]() { to_manual(true); });
        return;
    }
    m_found = res;
    spin->hide();
    const auto& c = res->country;
    const auto& r = c.regions[res->region_index];
    found->setText(QStringLiteral("%1 · %2").arg(locations::region_name(r, i18n::lang()), c.name(i18n::lang())));
    found_sub->setText(QStringLiteral("%1, %2  (%3)")
                           .arg(res->fix.lat, 0, 'f', 2)
                           .arg(res->fix.lng, 0, 'f', 2)
                           .arg((res->fix.source == QStringLiteral("gps")) ? QStringLiteral("GPS") : QStringLiteral("IP")));
    btnrow->show();
}

void Onboarding::accept_found() {
    if (!m_found.has_value()) return;
    const auto& c = m_found->country;
    int idx = m_found->region_index;
    const auto& fix = m_found->fix;
    s->country_code = c.code;
    s->region_index = idx;
    s->lat = fix.lat;
    s->lng = fix.lng;
    s->tz = c.regions[idx].tz;
    s->auto_location = true;
    fill_methods();
    go(4);
}

void Onboarding::to_manual(bool from_auto) {
    mode = QStringLiteral("manual");
    s->auto_location = false;
    if (from_auto && m_found.has_value()) {
        s->country_code = m_found->country.code;
        s->region_index = m_found->region_index;
        cb_country->set_value(s->country_code);
        fill_regions();
    }
    go(3);
}

void Onboarding::after_location() {
    auto c = locations::by_code(s->country_code);
    int idx = std::min(std::max(s->region_index, 0), static_cast<int>(c.regions.size()) - 1);
    const auto& r = c.regions[idx];
    s->lat = r.lat;
    s->lng = r.lng;
    s->tz = r.tz;
    fill_methods();
    go(4);
}

void Onboarding::to_done() {
    go(6);
    QTimer::singleShot(350, mark, &SuccessMark::play);
}

void Onboarding::do_finish() {
    s->first_run = false;
    s->save();
    emit finished();
}

} // namespace vm::ui
