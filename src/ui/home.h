#pragma once
#include "../notifier.h"
#include "../state.h"
#include "widgets.h"
#include <QAbstractButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QPixmap>
#include <QRectF>
#include <QScrollArea>
#include <QTimer>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>
#include <tuple>

namespace vm::ui {

QPainterPath arch_path(const QRectF& r, double radius = 26.0);

class PrayerCard : public QAbstractButton {
    Q_OBJECT
public:
    QString key;
    QString time_text;
    QString state; // next | past | future
    QString lang;
    bool alarm_on = true;

    PrayerCard(
        const QString& key_,
        const QString& time_text_,
        const QString& state_,
        const QString& lang_,
        bool alarm_on_ = true,
        QWidget* parent = nullptr
    );

    void set_glow(double v);

signals:
    void alarmToggled(const QString& key);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void render_now();

private:
    void hv(double v);
    QRectF art_rect() const;
    QRectF bell_rect() const;

    double m_h = 0.0;
    QVariantAnimation* m_ha = nullptr;
    QPixmap m_pm;
    QTimer* m_rerender = nullptr;
    double m_glow = 0.0;
};

class GlassChip : public QWidget {
    Q_OBJECT
public:
    QString icon_name;
    double fs = 10.5;
    int fw = 700;
    bool accent = false;
    QString text;

    explicit GlassChip(const QString& icon = QString(), double size = 10.5, int weight = 700, bool accent_ = false, QWidget* parent = nullptr);
    void set_text(const QString& txt);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class CountdownBlock : public QWidget {
    Q_OBJECT
public:
    static constexpr int W = 372;
    static constexpr int H = 178;

    std::tuple<int, int, int> hms{0, 0, 0};
    double frac = 0.0;
    QString caption;
    QString prev_name;
    QString next_name;

    explicit CountdownBlock(QWidget* parent = nullptr);
    void set_values(std::tuple<int, int, int> hms_, double frac_, const QString& cap, const QString& prev, const QString& next);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class HeroCard : public QFrame {
    Q_OBJECT
public:
    AppState* state = nullptr;
    QString key = QStringLiteral("fajr");
    GlassChip* pill = nullptr;
    Label* name = nullptr;
    GlassChip* chip = nullptr;
    CountdownBlock* countdown = nullptr;

    explicit HeroCard(AppState* st, QWidget* parent = nullptr);
    void set_prayer(const QString& key_, const QString& time_text);

protected:
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void render_now();

private:
    QPixmap m_pm;
    QString m_pm_key;
    QTimer* m_rerender = nullptr;
};

class QLabelIcon : public QWidget {
    Q_OBJECT
public:
    QString name;
    QString color;
    QLabelIcon(const QString& name_, int size, const QString& color_ = QStringLiteral("accent"), QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class HomePage : public QWidget {
    Q_OBJECT
public:
    AppState* state = nullptr;
    Notifier* notifier = nullptr;
    QLabelIcon* pin = nullptr;
    Label* city = nullptr;
    Label* dateline = nullptr;
    Button* snooze_chip = nullptr;
    Segmented* lang = nullptr;
    IconButton* theme_btn = nullptr;
    HeroCard* hero = nullptr;
    QScrollArea* scroll = nullptr;
    QWidget* row_host = nullptr;
    QHBoxLayout* row = nullptr;

    HomePage(AppState* st, Notifier* notif, QWidget* parent = nullptr);

    void restyle();
    void refresh();
    void tick();

signals:
    void openSettings();
    void languageChanged(const QString& code);
    void themeToggle();
    void snoozeCancel();

private slots:
    void sync_bells();
    void toggle_alarm(const QString& key);
    void on_glow(const QVariant& v);

private:
    QVector<PrayerCard*> m_cards;
    QString m_last_next;
    QVariantAnimation* m_glow_a = nullptr;
};

} // namespace vm::ui
