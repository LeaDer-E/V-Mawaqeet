#pragma once
#include <QAbstractButton>
#include <QColor>
#include <QFont>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>
#include <optional>
#include <utility>

namespace vm::ui {

inline const QString FAMILY = QStringLiteral("Tajawal");

QFont font(double size = 10.5, int weight = 500);

class Card : public QFrame {
    Q_OBJECT
public:
    int radius = 22;
    explicit Card(int rad = 22, QWidget* parent = nullptr, int pad = 0);

protected:
    void paintEvent(QPaintEvent* e) override;
};

class Label : public QLabel {
    Q_OBJECT
public:
    explicit Label(
        const QString& text = QString(),
        double size = 11.0,
        int weight = 500,
        const QString& color = QStringLiteral("text"),
        QWidget* parent = nullptr,
        std::optional<Qt::Alignment> align = std::nullopt
    );

    void restyle();
    void set_color(const QString& c);

private:
    QString m_color;
    double m_size;
    int m_weight;
};

class Button : public QAbstractButton {
    Q_OBJECT
public:
    QString kind;
    QString icon_name;

    explicit Button(
        const QString& text = QString(),
        const QString& kind_ = QStringLiteral("primary"),
        const QString& icon = QString(),
        QWidget* parent = nullptr,
        int height = 44
    );

    QSize sizeHint() const override;
    void set_busy(bool b);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void tick();

private:
    void hover_to(double v);
    double m_h = 0.0;
    QVariantAnimation* m_ha = nullptr;
    bool m_busy = false;
    double m_spin = 0.0;
    QTimer* m_timer = nullptr;
};

class IconButton : public QAbstractButton {
    Q_OBJECT
public:
    QString icon_name;
    int radius = 12;
    QString color_name = QStringLiteral("text2");
    int glyph = 18;
    bool danger = false;
    std::optional<QColor> fixed_color;

    explicit IconButton(
        const QString& icon,
        int size = 40,
        const QString& tip = QString(),
        QWidget* parent = nullptr,
        int radius_ = 12,
        const QString& color = QStringLiteral("text2"),
        int glyph_ = 18
    );

    void set_icon(const QString& name);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void hover_to(double v);
    double m_h = 0.0;
    QVariantAnimation* m_ha = nullptr;
};

class Toggle : public QAbstractButton {
    Q_OBJECT
public:
    explicit Toggle(bool checked = false, QWidget* parent = nullptr);
    void set_value(bool v);

signals:
    void toggled_by_user(bool checked);

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void on_clicked();

private:
    void go(double v);
    double m_x = 0.0;
    QVariantAnimation* m_a = nullptr;
};

class Segmented : public QWidget {
    Q_OBJECT
public:
    QVector<std::pair<QString, QString>> items;
    QString current;
    QMap<QString, QString> icons_map;

    Segmented(
        const QVector<std::pair<QString, QString>>& items_,
        const QString& current_,
        QWidget* parent = nullptr,
        const QMap<QString, QString>& icons_ = {},
        int height = 42,
        int min_seg = 0
    );

    QSize sizeHint() const override;
    void set_current(const QString& key, bool animate = true);

signals:
    void changed(const QString& key);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    int current_index() const;
    double slot_pos(double i) const;
    double m_pos = 0.0;
    QVariantAnimation* m_a = nullptr;
    int m_min_seg = 0;
};

class Stepper : public QWidget {
    Q_OBJECT
public:
    int value = 0;
    int lo = -30;
    int hi = 30;

    explicit Stepper(int val = 0, int lo_ = -30, int hi_ = 30, QWidget* parent = nullptr);
    void set_value(int v);

signals:
    void changed(int val);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
};

class MinutesInput : public QWidget {
    Q_OBJECT
public:
    int lo = 0;
    int hi = 600;
    QLineEdit* edit = nullptr;

    explicit MinutesInput(
        int value = 5,
        int lo_ = 0,
        int hi_ = 600,
        const QString& unit = QString(),
        QWidget* parent = nullptr,
        bool compact = false
    );

    int value() const;
    void set_value(int v);

signals:
    void changed(int val);

private slots:
    void on_tc(const QString& txt);
    void on_fin();
};

class StatusBadge : public QWidget {
    Q_OBJECT
public:
    QString text;
    QString kind;

    explicit StatusBadge(const QString& txt = QString(), const QString& knd = QStringLiteral("ok"), QWidget* parent = nullptr);
    void set(const QString& txt, const QString& knd);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* e) override;
};

struct ComboItem {
    QString label;
    QString sublabel;
    QVariant value;
    QString haystack;
};

class SearchPopup : public QFrame {
    Q_OBJECT
public:
    QVector<ComboItem> items;
    QString no_results;
    QLineEdit* edit = nullptr;
    QListWidget* list = nullptr;

    SearchPopup(const QVector<ComboItem>& items_, const QString& placeholder, const QString& no_res, QWidget* parent = nullptr);
    void select_value(const QVariant& val);

signals:
    void picked(const QVariant& val);

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void fill(const QString& q);
    void click_item(QListWidgetItem* it);
};

class SearchCombo : public QAbstractButton {
    Q_OBJECT
public:
    QVector<ComboItem> items;
    QVariant value;
    QString placeholder;
    QString no_results;

    explicit SearchCombo(const QString& placeholder_ = QString(), const QString& no_results_ = QString(), QWidget* parent = nullptr);
    void set_items(const QVector<ComboItem>& items_);
    void set_value(const QVariant& v);

signals:
    void changed(const QVariant& val);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void open_popup();
    void on_picked(const QVariant& v);

private:
    QString current_label() const;
    void hover_to(double v);
    double m_h = 0.0;
    QVariantAnimation* m_ha = nullptr;
    SearchPopup* m_pop = nullptr;
};

class Toast : public QWidget {
    Q_OBJECT
public:
    QString text;
    QString kind = QStringLiteral("ok");

    explicit Toast(QWidget* parent);
    void show_msg(const QString& txt, const QString& knd = QStringLiteral("ok"), int ms = 2600);

protected:
    void paintEvent(QPaintEvent* e) override;

private slots:
    void fade_out();

private:
    void anim_to(double v);
    QGraphicsOpacityEffect* m_fx = nullptr;
    QTimer* m_t = nullptr;
    QPropertyAnimation* m_a = nullptr;
};

class SlideStack : public QWidget {
    Q_OBJECT
public:
    QVector<QWidget*> pages;
    int index = -1;

    explicit SlideStack(QWidget* parent = nullptr);
    int add(QWidget* w);
    void go(int i, int direction = 1, bool animate = true);

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    QParallelAnimationGroup* m_grp = nullptr;
};

} // namespace vm::ui
