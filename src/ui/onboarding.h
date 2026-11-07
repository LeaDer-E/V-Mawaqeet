#pragma once
#include "../state.h"
#include "locate.h"
#include "widgets.h"
#include <QAbstractButton>
#include <QTimer>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>
#include <functional>
#include <optional>

namespace vm::ui {

class OptionCard : public QAbstractButton {
    Q_OBJECT
public:
    QString icon_name;
    QString title;
    QString sub;
    bool sel = false;
    QString glyph_text;

    OptionCard(const QString& icon, const QString& title_, const QString& sub_ = QString(), bool selected = false, const QString& glyph = QString(), QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void go(double v);
    double m_h = 0.0;
    QVariantAnimation* m_a = nullptr;
};

class ThemeCard : public QAbstractButton {
    Q_OBJECT
public:
    QString mode;
    QString title;
    bool sel = false;

    ThemeCard(const QString& mode_, const QString& title_, bool selected, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    static void draw_mini(QPainter* p, const QRectF& r, bool dark);
};

class Spinner : public QWidget {
    Q_OBJECT
public:
    int a = 0;
    explicit Spinner(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class SuccessMark : public QWidget {
    Q_OBJECT
public:
    double v = 0.0;
    explicit SuccessMark(QWidget* parent = nullptr);
    void play();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QVariantAnimation* m_an = nullptr;
};

class Dots : public QWidget {
    Q_OBJECT
public:
    int i = 0;
    explicit Dots(QWidget* parent = nullptr);
    void set(int idx);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class Onboarding : public QWidget {
    Q_OBJECT
public:
    AppState* state = nullptr;
    Locator* locator = nullptr;
    Settings* s = nullptr;
    QString mode = QStringLiteral("manual");
    Dots* dots = nullptr;
    SlideStack* stack = nullptr;

    Spinner* spin = nullptr;
    Label* found = nullptr;
    Label* found_sub = nullptr;
    Button* yes = nullptr;
    Button* edit = nullptr;
    QWidget* btnrow = nullptr;
    SearchCombo* cb_country = nullptr;
    SearchCombo* cb_region = nullptr;
    SearchCombo* cb_method = nullptr;
    SuccessMark* mark = nullptr;

    Onboarding(AppState* st, Locator* loc, QWidget* parent = nullptr);

    void build(int start = 0, bool animate = true);
    void go(int i, int direction = 1, bool animate = true);

signals:
    void finished();
    void languageChanged(const QString& code);
    void themeChanged(const QString& mode);

private:
    QWidget* make_page(
        const QString& title,
        const QString& sub,
        QWidget* content,
        std::optional<int> back = std::nullopt,
        const QString& nxt_label = QString(),
        std::function<void()> nxt_cb = nullptr
    );
    QWidget* make_cards(const QVector<QWidget*>& cards);
    void fill_regions();
    void fill_methods();
    void set_lang(const QString& code);
    void set_theme(const QString& k);
    void start_auto();
    void on_detected(std::optional<DetectedLocation> res);
    void accept_found();
    void to_manual(bool from_auto = false);
    void after_location();
    void to_done();
    void do_finish();

    std::optional<DetectedLocation> m_found;
    QVector<ThemeCard*> m_theme_cards;
};

} // namespace vm::ui
