#include "window.h"
#include "../app_info.h"
#include "../i18n.h"
#include "../theme.h"
#include "icons.h"
#include <QApplication>
#include <QBrush>
#include <QCloseEvent>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPropertyAnimation>
#include <QRadialGradient>
#include <QStyleHints>
#include <QVBoxLayout>
#include <QWindow>
#include <algorithm>
#include <cmath>

namespace vm::ui {

static constexpr int M = 10;
static constexpr int SIDE_W = 76;
static constexpr int SIDE_OPEN = 212;

// ---- TitleBar --------------------------------------------------------------
TitleBar::TitleBar(MainWindow* w) : QWidget(w), win(w) {
    setFixedHeight(40);
    setLayoutDirection(Qt::LeftToRight);
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(14, 4, 8, 0);
    lay->setSpacing(4);
    logo = new QLabel();
    logo->setPixmap(icons::logo_pixmap(22));
    logo->setFixedSize(22, 22);
    title = new QLabel(APP_NAME);
    title->setFont(ui::font(10.0, 700));
    lay->addWidget(logo);
    lay->addSpacing(6);
    lay->addWidget(title);
    lay->addStretch(1);

    b_min = new IconButton(QStringLiteral("minus"), 34, QString(), nullptr, 10, QStringLiteral("text2"), 15);
    b_max = new IconButton(QStringLiteral("square"), 34, QString(), nullptr, 10, QStringLiteral("text2"), 14);
    b_close = new IconButton(QStringLiteral("x"), 34, QString(), nullptr, 10, QStringLiteral("text2"), 15);
    b_close->danger = true;

    connect(b_min, &QAbstractButton::clicked, win, &QWidget::showMinimized);
    connect(b_max, &QAbstractButton::clicked, win, &MainWindow::toggle_max);
    connect(b_close, &QAbstractButton::clicked, win, &QWidget::close);
    lay->addWidget(b_min);
    lay->addWidget(b_max);
    lay->addWidget(b_close);
    restyle();
}

void TitleBar::restyle() {
    title->setStyleSheet(QStringLiteral("color:%1; background:transparent;").arg(theme::T().css(QStringLiteral("text2"))));
}

void TitleBar::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && !win->isMaximized() && win->windowHandle()) {
        win->windowHandle()->startSystemMove();
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent*) {
    win->toggle_max();
}

// ---- NavItem ---------------------------------------------------------------
NavItem::NavItem(const QString& icon, const QString& label_, QWidget* parent)
    : QAbstractButton(parent), icon_name(icon), label(label_) {
    setFixedHeight(50);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
}

void NavItem::enterEvent(QEnterEvent* e) {
    go(1.0);
    QAbstractButton::enterEvent(e);
}

void NavItem::leaveEvent(QEvent* e) {
    go(0.0);
    QAbstractButton::leaveEvent(e);
}

void NavItem::go(double v) {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = new QVariantAnimation(this);
    m_a->setStartValue(m_h);
    m_a->setEndValue(v);
    m_a->setDuration(160);
    connect(m_a, &QVariantAnimation::valueChanged, this, [this](const QVariant& x) {
        m_h = x.toDouble();
        update();
    });
    m_a->start();
}

void NavItem::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect()).adjusted(10, 2, -10, -2);
    QColor fg;
    if (active) {
        QLinearGradient g(r.topLeft(), r.bottomRight());
        g.setColorAt(0, T.c(QStringLiteral("accent")));
        g.setColorAt(1, T.c(QStringLiteral("accent2")));
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(g));
        p.drawRoundedRect(r, 15, 15);
        fg = T.c(QStringLiteral("on_accent"));
    } else {
        p.setPen(Qt::NoPen);
        QColor hov = T.dark() ? QColor(255, 255, 255, static_cast<int>(28 * m_h))
                              : QColor(27, 36, 32, static_cast<int>(18 * m_h));
        p.setBrush(hov);
        p.drawRoundedRect(r, 15, 15);
        fg = (m_h > 0.4) ? T.c(QStringLiteral("text")) : T.c(QStringLiteral("text2"));
    }
    bool rtl = isRightToLeft();
    QRectF ic(rtl ? (r.right() - 41.0) : (r.left() + 12.0), r.center().y() - 11.0, 22.0, 22.0);
    icons::draw(&p, icon_name, ic, fg, 1.9);
    if (width() > 120) {
        p.setPen(fg);
        p.setFont(ui::font(11.0, 700));
        QRectF tr = rtl ? QRectF(r.left() + 12, r.top(), r.width() - 62, r.height())
                        : QRectF(r.left() + 52, r.top(), r.width() - 62, r.height());
        p.drawText(tr, (rtl ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter | Qt::AlignAbsolute, label);
    }
}

// ---- Sidebar ---------------------------------------------------------------
Sidebar::Sidebar(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_Hover);
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 16, 0, 16);
    lay->setSpacing(6);
    logo = new QLabel();
    logo->setPixmap(icons::logo_pixmap(40));
    logo->setAlignment(Qt::AlignCenter);
    lay->addWidget(logo);
    lay->addSpacing(22);

    const std::tuple<QString, QString, QString> navs[] = {
        {QStringLiteral("home"), QStringLiteral("home"), i18n::t(QStringLiteral("home"))},
        {QStringLiteral("settings"), QStringLiteral("gear"), i18n::t(QStringLiteral("settings"))},
        {QStringLiteral("themes"), QStringLiteral("palette"), i18n::t(QStringLiteral("themes"))},
        {QStringLiteral("about"), QStringLiteral("info"), i18n::t(QStringLiteral("about"))}
    };
    for (const auto& [k, ic, lbl] : navs) {
        auto* it = new NavItem(ic, lbl);
        connect(it, &QAbstractButton::clicked, this, [this, k]() { emit navigate(k); });
        items.insert(k, it);
        lay->addWidget(it);
    }
    lay->addStretch(1);
    setFixedWidth(SIDE_W);
}

void Sidebar::set_active(const QString& k) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        it.value()->active = (it.key() == k);
        it.value()->update();
    }
}

void Sidebar::enterEvent(QEnterEvent* e) {
    animate_to(1.0);
    QWidget::enterEvent(e);
}

void Sidebar::leaveEvent(QEvent* e) {
    animate_to(0.0);
    QWidget::leaveEvent(e);
}

void Sidebar::animate_to(double v) {
    if (m_a) {
        m_a->stop();
        m_a->deleteLater();
    }
    m_a = new QVariantAnimation(this);
    m_a->setStartValue(expanded);
    m_a->setEndValue(v);
    m_a->setDuration(200);
    m_a->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_a, &QVariantAnimation::valueChanged, this, &Sidebar::on_val);
    m_a->start();
}

void Sidebar::on_val(const QVariant& v) {
    expanded = v.toDouble();
    int w = static_cast<int>(SIDE_W + (SIDE_OPEN - SIDE_W) * expanded);
    setFixedWidth(w);
    if (auto* win = qobject_cast<MainWindow*>(parentWidget())) {
        win->place_sidebar();
    }
}

void Sidebar::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRectF r = QRectF(rect());
    p.setPen(QPen(T.c(QStringLiteral("border")), 1));
    p.setBrush(T.dark() ? T.c(QStringLiteral("bg2")) : QColor(QStringLiteral("#FFFFFF")));
    bool rtl = isRightToLeft();
    p.drawRoundedRect(r.adjusted(!rtl ? -30 : 1, 0, !rtl ? -1 : 30, 0), 0, 0);
}

// ---- MainWindow ------------------------------------------------------------
MainWindow::MainWindow(const Settings& settings, bool start_hidden)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::Window) {
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setWindowTitle(APP_NAME);
    setWindowIcon(QIcon(icons::logo_pixmap(64)));
    setMinimumSize(900 + 2 * M, 600 + 2 * M);
    resize(1280, 820);

    state = new AppState(settings, this);
    s = &state->s;
    notifier = new Notifier(state, this);
    sound = new SoundPlayer(this);
    popup = new NotificationPopup();
    locator = new Locator(state, this);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(M, M, M, M);
    root->setSpacing(0);
    titlebar = new TitleBar(this);
    root->addWidget(titlebar);
    pages = new QStackedWidget();
    root->addWidget(pages, 1);

    sidebar = new Sidebar(this);
    connect(sidebar, &Sidebar::navigate, this, &MainWindow::on_nav);
    toast = new Toast(this);
    tray = make_tray();

    connect(state, &AppState::timesChanged, this, &MainWindow::on_times_changed);
    connect(state, &AppState::toast, this, &MainWindow::on_toast);
    connect(notifier, &Notifier::fire, this, &MainWindow::on_fire);
    connect(popup, &NotificationPopup::stopped, sound, &SoundPlayer::stop);
    connect(popup, &NotificationPopup::snoozed, this, [this](const QString& k, int m) {
        sound->stop();
        notifier->snooze(k, m);
    });
    connect(popup, &NotificationPopup::openApp, this, &MainWindow::bring_front);

    m_clock = new QTimer(this);
    m_clock->setInterval(1000);
    connect(m_clock, &QTimer::timeout, this, &MainWindow::on_tick);
    m_clock->start();

    sync_friday();
    apply_look(false);
    build_pages();
    if (s->first_run) {
        show_onboarding();
    } else {
        state->reload();
        show_page(QStringLiteral("home"), false);
    }
    if (!start_hidden) {
        show();
    }
}

QString MainWindow::effective_theme() const {
    QString th = s->theme;
    if (th == QStringLiteral("auto")) {
        if (auto* hints = QGuiApplication::styleHints()) {
            return (hints->colorScheme() == Qt::ColorScheme::Dark) ? QStringLiteral("dark") : QStringLiteral("light");
        }
        return QStringLiteral("dark");
    }
    return th;
}

void MainWindow::apply_look(bool rebuild_flag) {
    i18n::set_language(s->language);
    QApplication::setLayoutDirection(i18n::is_rtl() ? Qt::RightToLeft : Qt::LeftToRight);
    auto& T = theme::T();
    T.custom_accent = s->custom_accent;
    T.set_mode(effective_theme(), s->palette);
    if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
        app->setStyleSheet(T.qss());
    }
    titlebar->restyle();
    titlebar->logo->setPixmap(icons::logo_pixmap(22));
    setWindowIcon(QIcon(icons::logo_pixmap(64)));
    sidebar->logo->setPixmap(icons::logo_pixmap(40));
    if (tray) {
        tray->setIcon(QIcon(icons::logo_pixmap(64)));
    }
    update();
    if (rebuild_flag) {
        crossfade([this]() { rebuild(); });
    }
}

void MainWindow::crossfade(std::function<void()> fn) {
    QPixmap snap = grab();
    fn();
    auto* ov = new QLabel(this);
    ov->setPixmap(snap);
    ov->setGeometry(rect());
    ov->setAttribute(Qt::WA_TransparentForMouseEvents);
    ov->show();
    ov->raise();
    auto* fx = new QGraphicsOpacityEffect(ov);
    ov->setGraphicsEffect(fx);
    auto* a = new QPropertyAnimation(fx, "opacity", ov);
    a->setStartValue(1.0);
    a->setEndValue(0.0);
    a->setDuration(380);
    a->setEasingCurve(QEasingCurve::OutCubic);
    connect(a, &QPropertyAnimation::finished, ov, &QObject::deleteLater);
    a->start();
}

void MainWindow::set_language(const QString& code) {
    s->language = code;
    s->save();
    QTimer::singleShot(0, this, [this]() { apply_look(); });
}

void MainWindow::set_theme(const QString& mode) {
    s->theme = mode;
    s->save();
    QTimer::singleShot(0, this, [this]() { apply_look(); });
}

void MainWindow::on_nav(const QString& name) {
    if (name == QStringLiteral("about")) {
        show_about();
    } else {
        show_page(name);
    }
}

void MainWindow::show_about() {
    if (m_about && m_about->isVisible()) {
        return;
    }
    m_about = new AboutDialog(this, state->snapshot().next_key, [this](const QString& msg) {
        toast->show_msg(msg, QStringLiteral("ok"), 1600);
        toast->raise();
    });
    connect(m_about.data(), &AboutDialog::closed, this, [this]() { m_about = nullptr; });
    m_about->open();
    toast->raise();
}

void MainWindow::set_palette(const QString& name) {
    s->palette = name;
    s->save();
    QTimer::singleShot(0, this, [this]() { apply_look(); });
}

void MainWindow::set_custom_accent(const QString& hex_col) {
    s->custom_accent = hex_col;
    s->palette = QStringLiteral("custom");
    s->save();
    QTimer::singleShot(0, this, [this]() { apply_look(); });
}

void MainWindow::toggle_theme() {
    set_theme(theme::T().dark() ? QStringLiteral("light") : QStringLiteral("dark"));
}

void MainWindow::rebuild() {
    QString cur = m_cur;
    bool is_ob = (pages->currentWidget() == onboarding && onboarding != nullptr);
    build_pages();
    for (const QString& k : {QStringLiteral("home"), QStringLiteral("settings"), QStringLiteral("themes"), QStringLiteral("about")}) {
        if (sidebar->items.contains(k)) {
            sidebar->items[k]->label = i18n::t(k);
            sidebar->items[k]->update();
        }
    }
    tray_menu_refresh();
    if (is_ob) {
        pages->setCurrentWidget(onboarding);
    } else {
        show_page(cur, false);
    }
    place_sidebar();
}

void MainWindow::build_pages() {
    for (QWidget* w : {static_cast<QWidget*>(home), static_cast<QWidget*>(settings_page), static_cast<QWidget*>(themes_page), static_cast<QWidget*>(onboarding)}) {
        if (w) {
            pages->removeWidget(w);
            w->hide();
            w->setParent(nullptr);
            w->deleteLater();
        }
    }
    home = new HomePage(state, notifier);
    connect(home, &HomePage::languageChanged, this, &MainWindow::set_language);
    connect(home, &HomePage::themeToggle, this, &MainWindow::toggle_theme);
    connect(home, &HomePage::snoozeCancel, notifier, &Notifier::cancel_snooze);

    settings_page = new SettingsPage(state, notifier, locator);
    connect(settings_page, &SettingsPage::languageChanged, this, &MainWindow::set_language);
    connect(settings_page, &SettingsPage::themeChanged, this, &MainWindow::set_theme);
    connect(settings_page, &SettingsPage::previewSound, this, &MainWindow::preview_sound);
    connect(settings_page, &SettingsPage::openThemes, this, [this]() { show_page(QStringLiteral("themes")); });

    themes_page = new ThemesPage(s);
    connect(themes_page, &ThemesPage::modeChanged, this, &MainWindow::set_theme);
    connect(themes_page, &ThemesPage::paletteChanged, this, &MainWindow::set_palette);
    connect(themes_page, &ThemesPage::customChosen, this, &MainWindow::set_custom_accent);

    onboarding = new Onboarding(state, locator);
    connect(onboarding, &Onboarding::languageChanged, this, &MainWindow::ob_language);
    connect(onboarding, &Onboarding::themeChanged, this, &MainWindow::ob_theme);
    connect(onboarding, &Onboarding::finished, this, &MainWindow::ob_done);

    pages->addWidget(onboarding);
    pages->addWidget(home);
    pages->addWidget(settings_page);
    pages->addWidget(themes_page);

    for (QWidget* w : {static_cast<QWidget*>(home), static_cast<QWidget*>(settings_page), static_cast<QWidget*>(themes_page)}) {
        if (i18n::is_rtl()) {
            w->setContentsMargins(0, 0, SIDE_W, 0);
        } else {
            w->setContentsMargins(SIDE_W, 0, 0, 0);
        }
    }
}

void MainWindow::show_onboarding() {
    sidebar->hide();
    pages->setCurrentWidget(onboarding);
}

void MainWindow::ob_language(const QString& code) {
    s->language = code;
    QTimer::singleShot(0, this, [this, code]() {
        i18n::set_language(code);
        QApplication::setLayoutDirection((code == QStringLiteral("ar")) ? Qt::RightToLeft : Qt::LeftToRight);
        QString ob_th = s->theme;
        crossfade([this]() {
            build_pages();
            pages->setCurrentWidget(onboarding);
            onboarding->build(1);
        });
        s->theme = ob_th;
    });
}

void MainWindow::ob_theme(const QString& mode) {
    s->theme = mode;
    auto& T = theme::T();
    T.custom_accent = s->custom_accent;
    T.set_mode(effective_theme(), s->palette);
    if (auto* app = qobject_cast<QApplication*>(QApplication::instance())) {
        app->setStyleSheet(T.qss());
    }
    titlebar->restyle();
    update();
    for (QWidget* w : findChildren<QWidget*>()) {
        w->update();
    }
}

void MainWindow::ob_done() {
    s->save();
    state->reload();
    apply_look(true);
    show_page(QStringLiteral("home"), false);
}

void MainWindow::show_page(const QString& name, bool animate) {
    QWidget* w = home;
    if (name == QStringLiteral("settings")) w = settings_page;
    else if (name == QStringLiteral("themes")) w = themes_page;
    m_cur = name;
    sidebar->show();
    place_sidebar();
    sidebar->raise();
    toast->raise();
    sidebar->set_active(name);
    if (pages->currentWidget() == w) {
        return;
    }
    pages->setCurrentWidget(w);
    if (animate && w) {
        auto* fx = new QGraphicsOpacityEffect(w);
        w->setGraphicsEffect(fx);
        auto* a = new QPropertyAnimation(fx, "opacity", w);
        a->setStartValue(0.0);
        a->setEndValue(1.0);
        a->setDuration(260);
        connect(a, &QPropertyAnimation::finished, w, [w]() { w->setGraphicsEffect(nullptr); });
        a->start();
    }
}

void MainWindow::place_sidebar() {
    if (sidebar->isHidden()) {
        return;
    }
    int m = m_maximized ? 0 : M;
    int top = m + titlebar->height();
    int h = height() - top - m - 1;
    int w = sidebar->width();
    int x = i18n::is_rtl() ? (width() - m - w) : m;
    sidebar->setGeometry(x, top, w, h);
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    place_sidebar();
    QWidget::resizeEvent(e);
}

void MainWindow::on_toast(const QString& k, const QString& kind) {
    if (k.contains(QLatin1Char('|'))) {
        QString base = k.section(QLatin1Char('|'), 0, 0);
        QString prayer = k.section(QLatin1Char('|'), 1);
        toast->show_msg(i18n::t(base, {{QStringLiteral("name"), i18n::t(prayer)}}), kind);
    } else {
        toast->show_msg(i18n::t(k), kind);
    }
}

void MainWindow::on_times_changed() {
    sync_friday();
    if (home) home->refresh();
}

bool MainWindow::sync_friday() {
    bool fri = (state->now().date().dayOfWeek() == 5); // Friday is 5 in Qt (1=Mon..7=Sun)
    if (fri == i18n::is_friday()) {
        return false;
    }
    i18n::set_friday(fri);
    return true;
}

void MainWindow::on_tick() {
    if (sync_friday()) {
        if (home) home->refresh();
        tray_update();
    }
    m_tray_n += 1;
    if (tray && ((m_menu && m_menu->isVisible()) || m_tray_n % 15 == 0)) {
        tray_update();
    }
    if (home && pages->currentWidget() == home && isVisible() && !isMinimized()) {
        home->tick();
    }
}

void MainWindow::on_fire(const QString& key, const QString& kind, const QDateTime& dt) {
    int minutes = 0;
    QDateTime shown = dt;
    if (kind == QStringLiteral("pre")) {
        minutes = s->reminder_min.value(key, 10);
    } else if (kind == QStringLiteral("iqama")) {
        shown = dt.addSecs(static_cast<qint64>(s->iqama_min.value(key, 15)) * 60);
    }
    popup->present(key, shown.toString(QStringLiteral("HH:mm")), kind, minutes, s->snooze_minutes);
    sound->play(sound->path_for(*s, kind), (kind == QStringLiteral("pre") || kind == QStringLiteral("iqama")) ? 900 : 1800);
}

void MainWindow::preview_sound(const QString& kind) {
    sound->play(sound->path_for(*s, kind), 300);
    QTimer::singleShot(7000, sound, &SoundPlayer::stop);
}

// ---- System Tray -----------------------------------------------------------
QSystemTrayIcon* MainWindow::make_tray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return nullptr;
    }
    auto* tr = new QSystemTrayIcon(QIcon(icons::logo_pixmap(64)), this);
    tr->setToolTip(APP_NAME);
    connect(tr, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::Trigger || r == QSystemTrayIcon::DoubleClick) {
            bring_front();
        }
    });
    m_menu = new QMenu();
    connect(m_menu, &QMenu::aboutToShow, this, &MainWindow::tray_update);
    if (auto* hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this](auto) {
            tray_menu_refresh();
        });
    }
    tr->setContextMenu(m_menu);
    tr->show();
    tray = tr;
    tray_menu_refresh();
    return tr;
}

std::pair<Snapshot, QString> MainWindow::countdown_text() {
    Snapshot snap = state->snapshot();
    QDateTime now = state->now();
    qint64 secs = std::max<qint64>(0, now.secsTo(snap.next_dt));
    int h = static_cast<int>(secs / 3600);
    int rem = static_cast<int>(secs % 3600);
    int m = rem / 60;
    int sec = rem % 60;
    QString str = (h > 0)
                      ? QStringLiteral("%1:%2:%3").arg(h, 2, 10, QLatin1Char('0')).arg(m, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'))
                      : QStringLiteral("%1:%2").arg(m, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    return {snap, str};
}

QColor MainWindow::menu_fg() {
    bool dark = true;
    if (auto* hints = QGuiApplication::styleHints()) {
        dark = (hints->colorScheme() == Qt::ColorScheme::Dark);
    }
    return dark ? QColor(QStringLiteral("#E4E6EA")) : QColor(QStringLiteral("#3B4048"));
}

QIcon MainWindow::mi(const QString& name, std::optional<QColor> color) const {
    return QIcon(icons::pixmap(name, 18, color.value_or(menu_fg())));
}

QIcon MainWindow::blank_icon() {
    QPixmap pm(36, 36);
    pm.setDevicePixelRatio(2.0);
    pm.fill(Qt::transparent);
    return QIcon(pm);
}

void MainWindow::tray_menu_refresh() {
    if (!tray || !m_menu) return;
    m_menu->clear();
    m_tray_next = m_menu->addAction(mi(QStringLiteral("clock")), QString());
    connect(m_tray_next, &QAction::triggered, this, &MainWindow::bring_front);

    QMenu* sub = m_menu->addMenu(i18n::t(QStringLiteral("today_times")));
    sub->menuAction()->setIcon(mi(QStringLiteral("calendar")));
    m_tray_times.clear();
    for (const QString& k : PRAYER_KEYS) {
        QAction* act = sub->addAction(blank_icon(), QString());
        connect(act, &QAction::triggered, this, &MainWindow::bring_front);
        m_tray_times[k] = act;
    }

    m_menu->addSeparator();
    QAction* a = m_menu->addAction(QIcon(icons::logo_pixmap(36)), i18n::t(QStringLiteral("tray_open")));
    connect(a, &QAction::triggered, this, &MainWindow::bring_front);
    QFont f = a->font();
    f.setBold(true);
    a->setFont(f);

    const std::tuple<QString, QString, QString> navs[] = {
        {QStringLiteral("home"), QStringLiteral("home"), QStringLiteral("home")},
        {QStringLiteral("settings"), QStringLiteral("gear"), QStringLiteral("settings")},
        {QStringLiteral("themes"), QStringLiteral("palette"), QStringLiteral("themes")},
        {QStringLiteral("about"), QStringLiteral("info"), QStringLiteral("about")}
    };
    for (const auto& [key, icon, label] : navs) {
        QAction* act = m_menu->addAction(mi(icon), i18n::t(label));
        connect(act, &QAction::triggered, this, [this, key]() { open_page(key); });
    }

    m_menu->addSeparator();
    m_tray_alerts = m_menu->addAction(mi(QStringLiteral("bell")), QString());
    connect(m_tray_alerts, &QAction::triggered, this, &MainWindow::tray_toggle_alerts);
    m_tray_snooze = m_menu->addAction(mi(QStringLiteral("snooze")), i18n::t(QStringLiteral("cancel_snooze")));
    connect(m_tray_snooze, &QAction::triggered, notifier, &Notifier::cancel_snooze);
    m_tray_theme = m_menu->addAction(mi(QStringLiteral("sun")), QString());
    connect(m_tray_theme, &QAction::triggered, this, &MainWindow::toggle_theme);

    QAction* langAct = m_menu->addAction(mi(QStringLiteral("globe")), i18n::t(QStringLiteral("switch_lang")));
    connect(langAct, &QAction::triggered, this, [this]() {
        set_language((s->language == QStringLiteral("ar")) ? QStringLiteral("en") : QStringLiteral("ar"));
    });

    m_menu->addSeparator();
    QAction* q = m_menu->addAction(mi(QStringLiteral("power")), i18n::t(QStringLiteral("quit")));
    connect(q, &QAction::triggered, this, &MainWindow::quit_app);
    tray_update();
}

void MainWindow::tray_update() {
    if (!tray || !m_tray_next) return;
    auto [snap, left] = countdown_text();
    m_tray_next->setText(
        QStringLiteral("%1:  ").arg(i18n::t(QStringLiteral("next_prayer")))
        + i18n::t(QStringLiteral("tray_next"), {
            {QStringLiteral("name"), i18n::t(snap.next_key)},
            {QStringLiteral("time"), snap.next_dt.toString(QStringLiteral("HH:mm"))},
            {QStringLiteral("left"), left}
        })
    );
    QColor ok_green(QStringLiteral("#3DBE8B"));
    QColor accent = theme::T().c(QStringLiteral("accent"));
    for (const auto& [k, dt] : snap.today) {
        if (!m_tray_times.contains(k)) continue;
        QAction* act = m_tray_times[k];
        act->setText(QStringLiteral("%1\t%2").arg(i18n::t(k), dt.toString(QStringLiteral("HH:mm"))));
        if (k == snap.next_key) {
            act->setIcon(mi(QStringLiteral("clock"), accent));
        } else if (snap.past.contains(k)) {
            act->setIcon(mi(QStringLiteral("check"), ok_green));
        } else {
            act->setIcon(blank_icon());
        }
    }
    bool on = s->notify;
    m_tray_alerts->setText(i18n::t(on ? QStringLiteral("tray_alerts_on") : QStringLiteral("tray_alerts_off")));
    m_tray_alerts->setIcon(on ? mi(QStringLiteral("bell"), accent) : mi(QStringLiteral("bell_off")));
    m_tray_snooze->setVisible(notifier->next_snooze().has_value());
    m_tray_theme->setText(i18n::t(theme::T().dark() ? QStringLiteral("to_light") : QStringLiteral("to_dark")));
    m_tray_theme->setIcon(mi(theme::T().dark() ? QStringLiteral("sun") : QStringLiteral("moon")));
    tray->setToolTip(QStringLiteral("%1\n%2 %3 · %4")
                         .arg(APP_NAME, i18n::t(snap.next_key), snap.next_dt.toString(QStringLiteral("HH:mm")), left));
}

void MainWindow::tray_toggle_alerts() {
    tray_set_alerts(!s->notify);
}

void MainWindow::tray_set_alerts(bool on) {
    s->notify = on;
    s->save();
    emit state->alertsChanged();
    tray_update();
}

void MainWindow::open_page(const QString& name) {
    bring_front();
    if (pages->currentWidget() == onboarding) {
        return;
    }
    if (name == QStringLiteral("about")) {
        show_about();
    } else {
        show_page(name);
    }
}

void MainWindow::bring_front() {
    if (isMinimized()) showNormal();
    else show();
    raise();
    activateWindow();
}

void MainWindow::quit_app() {
    m_quitting = true;
    sound->stop();
    popup->hide();
    if (tray) tray->hide();
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent* e) {
    if (s->minimize_to_tray && tray && !m_quitting) {
        e->ignore();
        hide();
        return;
    }
    quit_app();
    e->accept();
}

void MainWindow::toggle_max() {
    if (isMaximized()) showNormal();
    else showMaximized();
}

void MainWindow::changeEvent(QEvent* e) {
    QWidget::changeEvent(e);
    if (e->type() == QEvent::WindowStateChange) {
        m_maximized = isMaximized();
        int m = m_maximized ? 0 : M;
        if (layout()) layout()->setContentsMargins(m, m, m, m);
        titlebar->b_max->set_icon(m_maximized ? QStringLiteral("restore") : QStringLiteral("square"));
        place_sidebar();
        update();
    }
}

Qt::Edges MainWindow::edges_at(const QPoint& pos) const {
    if (m_maximized) return Qt::Edges();
    Qt::Edges e;
    QRect r = rect();
    int g = M + 2;
    if (pos.x() < g) e |= Qt::LeftEdge;
    if (pos.x() > r.width() - g) e |= Qt::RightEdge;
    if (pos.y() < g) e |= Qt::TopEdge;
    if (pos.y() > r.height() - g) e |= Qt::BottomEdge;
    return e;
}

void MainWindow::mouseMoveEvent(QMouseEvent* ev) {
    Qt::Edges ed = edges_at(ev->position().toPoint());
    Qt::CursorShape c = Qt::ArrowCursor;
    if (ed == (Qt::LeftEdge | Qt::TopEdge) || ed == (Qt::RightEdge | Qt::BottomEdge)) {
        c = Qt::SizeFDiagCursor;
    } else if (ed == (Qt::RightEdge | Qt::TopEdge) || ed == (Qt::LeftEdge | Qt::BottomEdge)) {
        c = Qt::SizeBDiagCursor;
    } else if (ed & (Qt::LeftEdge | Qt::RightEdge)) {
        c = Qt::SizeHorCursor;
    } else if (ed & (Qt::TopEdge | Qt::BottomEdge)) {
        c = Qt::SizeVerCursor;
    }
    setCursor(c);
}

void MainWindow::mousePressEvent(QMouseEvent* ev) {
    Qt::Edges ed = edges_at(ev->position().toPoint());
    if (ev->button() == Qt::LeftButton && ed != Qt::Edges() && windowHandle()) {
        windowHandle()->startSystemResize(ed);
    }
}

void MainWindow::paintEvent(QPaintEvent*) {
    auto& T = theme::T();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int m = m_maximized ? 0 : M;
    QRectF r = QRectF(rect()).adjusted(m, m, -m, -m);
    double rad = m_maximized ? 0.0 : 20.0;
    if (!m_maximized) {
        for (int i = 0; i < m; ++i) {
            p.setPen(Qt::NoPen);
            double ratio = 1.0 - static_cast<double>(i) / m;
            p.setBrush(QColor(0, 0, 0, static_cast<int>((T.dark() ? 34 : 22) * ratio * ratio)));
            p.drawRoundedRect(r.adjusted(-i, -i + 3, i, i + 3), rad + i, rad + i);
        }
    }
    QLinearGradient g(r.topLeft(), r.bottomRight());
    g.setColorAt(0, T.c(QStringLiteral("bg0")));
    g.setColorAt(1, T.c(QStringLiteral("bg1")));
    p.setPen(!m_maximized ? QPen(T.c(QStringLiteral("border_hi")), 1) : Qt::NoPen);
    p.setBrush(QBrush(g));
    p.drawRoundedRect(r, rad, rad);

    // ambient gold glow in the corner
    p.save();
    QPainterPath path;
    path.addRoundedRect(r, rad, rad);
    p.setClipPath(path);
    QRadialGradient rg(r.right() - 120, r.top() + 40, r.width() * 0.55);
    rg.setColorAt(0, T.c(QStringLiteral("accent"), T.dark() ? 0.10 : 0.12));
    rg.setColorAt(1, QColor(0, 0, 0, 0));
    p.fillRect(r, QBrush(rg));
    p.restore();
}

} // namespace vm::ui
