#pragma once
#include "../config.h"
#include "../notifier.h"
#include "../sound.h"
#include "../state.h"
#include "about_dialog.h"
#include "home.h"
#include "locate.h"
#include "notification.h"
#include "onboarding.h"
#include "settings_page.h"
#include "themes_page.h"
#include "widgets.h"
#include <QAbstractButton>
#include <QAction>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QPointer>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>
#include <functional>

namespace vm::ui {

class MainWindow;

class TitleBar : public QWidget {
    Q_OBJECT
public:
    MainWindow* win = nullptr;
    QLabel* logo = nullptr;
    QLabel* title = nullptr;
    IconButton* b_min = nullptr;
    IconButton* b_max = nullptr;
    IconButton* b_close = nullptr;

    explicit TitleBar(MainWindow* w);
    void restyle();

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
};

class NavItem : public QAbstractButton {
    Q_OBJECT
public:
    QString icon_name;
    QString label;
    bool active = false;

    NavItem(const QString& icon, const QString& label_, QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void go(double v);
    double m_h = 0.0;
    QVariantAnimation* m_a = nullptr;
};

class Sidebar : public QWidget {
    Q_OBJECT
public:
    double expanded = 0.0;
    QLabel* logo = nullptr;
    QMap<QString, NavItem*> items;

    explicit Sidebar(QWidget* parent);
    void set_active(const QString& k);

signals:
    void navigate(const QString& key);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void animate_to(double v);
    void on_val(const QVariant& v);
    QVariantAnimation* m_a = nullptr;
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    AppState* state = nullptr;
    Settings* s = nullptr;
    Notifier* notifier = nullptr;
    SoundPlayer* sound = nullptr;
    NotificationPopup* popup = nullptr;
    Locator* locator = nullptr;

    TitleBar* titlebar = nullptr;
    QStackedWidget* pages = nullptr;
    Sidebar* sidebar = nullptr;
    Toast* toast = nullptr;

    HomePage* home = nullptr;
    SettingsPage* settings_page = nullptr;
    ThemesPage* themes_page = nullptr;
    Onboarding* onboarding = nullptr;
    QSystemTrayIcon* tray = nullptr;

    explicit MainWindow(const Settings& settings, bool start_hidden = false);

    void apply_look(bool rebuild = true);
    void set_language(const QString& code);
    void set_theme(const QString& mode);
    void set_palette(const QString& name);
    void set_custom_accent(const QString& hex_col);
    void toggle_theme();

    void show_onboarding();
    void show_page(const QString& name, bool animate = true);
    void show_about();
    void place_sidebar();
    void open_page(const QString& name);
    void bring_front();
    void quit_app();
    void toggle_max();
    void preview_sound(const QString& kind = QStringLiteral("adhan"));
    void tray_menu_refresh();

protected:
    void resizeEvent(QResizeEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    void changeEvent(QEvent* e) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void on_nav(const QString& name);
    void on_toast(const QString& k, const QString& kind);
    void on_times_changed();
    void on_tick();
    void on_fire(const QString& key, const QString& kind, const QDateTime& dt);
    void tray_update();
    void tray_toggle_alerts();

private:
    QString effective_theme() const;
    void crossfade(std::function<void()> fn);
    void rebuild();
    void build_pages();
    void ob_language(const QString& code);
    void ob_theme(const QString& mode);
    void ob_done();
    bool sync_friday();
    QSystemTrayIcon* make_tray();
    std::pair<Snapshot, QString> countdown_text();
    static QColor menu_fg();
    QIcon mi(const QString& name, std::optional<QColor> color = std::nullopt) const;
    static QIcon blank_icon();
    void tray_set_alerts(bool on);
    Qt::Edges edges_at(const QPoint& pos) const;

    bool m_maximized = false;
    bool m_quitting = false;
    QString m_cur = QStringLiteral("home");
    QPointer<AboutDialog> m_about;
    QTimer* m_clock = nullptr;
    int m_tray_n = 0;

    QMenu* m_menu = nullptr;
    QAction* m_tray_next = nullptr;
    QMap<QString, QAction*> m_tray_times;
    QAction* m_tray_alerts = nullptr;
    QAction* m_tray_snooze = nullptr;
    QAction* m_tray_theme = nullptr;
};

} // namespace vm::ui
