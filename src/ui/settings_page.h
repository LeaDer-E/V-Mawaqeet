#pragma once
#include "../notifier.h"
#include "../state.h"
#include "locate.h"
#include "widgets.h"
#include <QGridLayout>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <functional>

namespace vm::ui {

class Row : public QWidget {
    Q_OBJECT
public:
    Label* l = nullptr;
    Row(const QString& label, QWidget* control, const QString& sub = QString(), bool stack = false, QWidget* parent = nullptr);
};

class Section : public Card {
    Q_OBJECT
public:
    QVBoxLayout* body = nullptr;
    Label* title = nullptr;
    explicit Section(const QString& title_, QWidget* parent = nullptr);
    void add_widget(QWidget* w);
    void add_layout(QLayout* l);
};

class SoundRow : public QWidget {
    Q_OBJECT
public:
    Settings* s = nullptr;
    QString attr;
    QString kind;
    std::function<void()> save_cb;
    Button* btn = nullptr;
    IconButton* play = nullptr;
    IconButton* reset = nullptr;

    SoundRow(
        const QString& label,
        Settings* settings,
        const QString& attr_,
        const QString& kind_,
        std::function<void(const QString&)> preview_cb,
        std::function<void()> save_cb_,
        QWidget* parent = nullptr
    );

private slots:
    void choose();
    void do_reset();

private:
    QString get_attr() const;
    void set_attr(const QString& val);
    void refresh();
};

class SettingsPage : public QWidget {
    Q_OBJECT
public:
    AppState* state = nullptr;
    Notifier* notifier = nullptr;
    Locator* locator = nullptr;
    Settings* s = nullptr;

    Label* heading = nullptr;
    QScrollArea* scroll = nullptr;
    QWidget* host = nullptr;
    QGridLayout* grid = nullptr;
    QVector<Section*> sections;

    Toggle* auto_toggle = nullptr;
    Button* detect_btn = nullptr;
    Label* detect_info = nullptr;
    SearchCombo* country = nullptr;
    SearchCombo* region = nullptr;
    MinutesInput* sn_in = nullptr;
    SearchCombo* method = nullptr;
    Button* refresh_btn = nullptr;
    QMap<QString, StatusBadge*> badges;
    QMap<QString, Toggle*> alarm_t;

    SettingsPage(AppState* st, Notifier* notif, Locator* loc, QWidget* parent = nullptr);

signals:
    void languageChanged(const QString& code);
    void themeChanged(const QString& mode);
    void previewSound(const QString& kind);
    void openThemes();

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void update_status();
    void sync_flags();

private:
    void flow();
    void save();
    Section* make_location();
    Section* make_alerts();
    Section* make_prayers();
    Section* make_sources();
    Section* make_look();
    Section* make_system();

    void fill_regions();
    void set_manual_enabled();
    void auto_toggled(bool v);
    void detect();
    void country_changed(const QVariant& code);
    void region_changed(const QVariant& idx);
    void fill_methods();

    static QWidget* make_hdr(const QString& title, const QString& sub = QString());
    static QWidget* make_cell(Toggle* toggle, MinutesInput* minutes);

    int m_cols = 0;
    QVector<QVBoxLayout*> m_col_layouts;
};

} // namespace vm::ui
