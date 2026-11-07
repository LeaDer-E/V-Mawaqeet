#include "settings_page.h"
#include "../autostart.h"
#include "../config.h"
#include "../i18n.h"
#include "../locations.h"
#include "../methods.h"
#include "../theme.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <algorithm>

namespace vm::ui {

Row::Row(const QString& label, QWidget* control, const QString& sub, bool stack, QWidget* parent)
    : QWidget(parent) {
    QBoxLayout* lay = stack ? static_cast<QBoxLayout*>(new QVBoxLayout(this))
                            : static_cast<QBoxLayout*>(new QHBoxLayout(this));
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(stack ? 8 : 14);
    auto* col = new QVBoxLayout();
    col->setSpacing(0);
    l = new Label(label, 11.0, 600, QStringLiteral("text"));
    col->addWidget(l);
    if (!sub.isEmpty()) {
        col->addWidget(new Label(sub, 9.5, 500, QStringLiteral("text2")));
    }
    lay->addLayout(col, stack ? 0 : 1);
    lay->addWidget(control, stack ? 1 : 0);
    if (!stack) {
        lay->setAlignment(control, Qt::AlignVCenter);
    }
}

Section::Section(const QString& title_, QWidget* parent)
    : Card(24, parent) {
    body = new QVBoxLayout(this);
    body->setContentsMargins(26, 22, 26, 24);
    body->setSpacing(16);
    title = new Label(title_, 14.0, 800, QStringLiteral("accent"));
    body->addWidget(title);
}

void Section::add_widget(QWidget* w) {
    body->addWidget(w);
}

void Section::add_layout(QLayout* l) {
    body->addLayout(l);
}

// ---- SoundRow --------------------------------------------------------------
SoundRow::SoundRow(
    const QString& label,
    Settings* settings,
    const QString& attr_,
    const QString& kind_,
    std::function<void(const QString&)> preview_cb,
    std::function<void()> save_cb_,
    QWidget* parent
)
    : QWidget(parent), s(settings), attr(attr_), kind(kind_), save_cb(std::move(save_cb_)) {
    auto* col = new QVBoxLayout(this);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(8);
    col->addWidget(new Label(label, 11.0, 600, QStringLiteral("text")));

    auto* row = new QHBoxLayout();
    row->setSpacing(8);
    btn = new Button(QString(), QStringLiteral("ghost"), QStringLiteral("folder"), nullptr, 42);
    connect(btn, &QAbstractButton::clicked, this, &SoundRow::choose);
    play = new IconButton(QStringLiteral("play"), 42, QString(), nullptr, 13);
    connect(play, &QAbstractButton::clicked, this, [this, preview_cb]() {
        if (preview_cb) preview_cb(kind);
    });
    reset = new IconButton(QStringLiteral("refresh"), 42, i18n::t(QStringLiteral("reset")), nullptr, 13);
    connect(reset, &QAbstractButton::clicked, this, &SoundRow::do_reset);
    row->addWidget(btn, 1);
    row->addWidget(play);
    row->addWidget(reset);
    col->addLayout(row);
    refresh();
}

QString SoundRow::get_attr() const {
    if (attr == QStringLiteral("pre_sound")) return s->pre_sound;
    if (attr == QStringLiteral("iqama_sound")) return s->iqama_sound;
    return s->adhan_sound;
}

void SoundRow::set_attr(const QString& val) {
    if (attr == QStringLiteral("pre_sound")) s->pre_sound = val;
    else if (attr == QStringLiteral("iqama_sound")) s->iqama_sound = val;
    else s->adhan_sound = val;
}

void SoundRow::refresh() {
    QString path = get_attr();
    btn->setText(!path.isEmpty() ? QFileInfo(path).fileName() : i18n::t(QStringLiteral("default_sound")));
    reset->setVisible(!path.isEmpty());
}

void SoundRow::choose() {
    QString f = QFileDialog::getOpenFileName(this, i18n::t(QStringLiteral("choose_file")), QString(),
                                             QStringLiteral("Audio (*.mp3 *.wav *.ogg *.m4a *.flac *.aac)"));
    if (!f.isEmpty()) {
        set_attr(f);
        if (save_cb) save_cb();
        refresh();
    }
}

void SoundRow::do_reset() {
    set_attr(QString());
    if (save_cb) save_cb();
    refresh();
}

// ---- SettingsPage ----------------------------------------------------------
SettingsPage::SettingsPage(AppState* st, Notifier* notif, Locator* loc, QWidget* parent)
    : QWidget(parent), state(st), notifier(notif), locator(loc), s(&st->s) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(30, 14, 30, 0);
    outer->setSpacing(14);
    heading = new Label(i18n::t(QStringLiteral("settings")), 27.0, 800, QStringLiteral("text"));
    outer->addWidget(heading);

    scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    host = new QWidget();
    grid = new QGridLayout(host);
    grid->setContentsMargins(0, 4, 8, 28);
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(18);
    scroll->setWidget(host);
    outer->addWidget(scroll, 1);

    sections = {make_location(), make_alerts(), make_prayers(), make_sources(), make_look(), make_system()};
    flow();
    connect(state, &AppState::timesChanged, this, &SettingsPage::update_status);
    update_status();
}

void SettingsPage::resizeEvent(QResizeEvent* e) {
    flow();
    QWidget::resizeEvent(e);
}

void SettingsPage::flow() {
    int cols = (width() >= 1040) ? 2 : 1;
    if (cols == m_cols) {
        return;
    }
    m_cols = cols;
    for (Section* sec : sections) {
        sec->setParent(host);
    }
    while (QLayoutItem* it = grid->takeAt(0)) {
        if (QWidget* w = it->widget()) {
            w->hide();
            w->deleteLater();
        }
        delete it;
    }
    m_col_layouts.clear();

    QVector<QVector<Section*>> groups;
    if (cols == 2) {
        groups.append(QVector<Section*>{sections[0], sections[1]});
        groups.append(QVector<Section*>{sections[2], sections[3], sections[4], sections[5]});
    } else {
        groups.append(sections);
    }

    for (int ci = 0; ci < groups.size(); ++ci) {
        auto* colWidget = new QWidget(host);
        auto* col = new QVBoxLayout(colWidget);
        col->setContentsMargins(0, 0, 0, 0);
        col->setSpacing(18);
        for (Section* sec : groups[ci]) {
            col->addWidget(sec);
            sec->show();
        }
        col->addStretch(1);
        grid->addWidget(colWidget, 0, ci);
        grid->setColumnStretch(ci, 1);
    }
    for (int c = static_cast<int>(groups.size()); c < 2; ++c) {
        grid->setColumnStretch(c, 0);
    }
}

void SettingsPage::save() {
    s->save();
}

Section* SettingsPage::make_location() {
    auto* sec = new Section(i18n::t(QStringLiteral("sec_location")));
    auto_toggle = new Toggle(s->auto_location);
    connect(auto_toggle, &Toggle::toggled_by_user, this, &SettingsPage::auto_toggled);
    sec->add_widget(new Row(i18n::t(QStringLiteral("auto_loc")), auto_toggle));

    detect_btn = new Button(i18n::t(QStringLiteral("detect_now")), QStringLiteral("ghost"), QStringLiteral("locate"), nullptr, 42);
    connect(detect_btn, &QAbstractButton::clicked, this, &SettingsPage::detect);
    sec->add_widget(detect_btn);

    detect_info = new Label(QString(), 10.0, 600, QStringLiteral("accent"));
    detect_info->setWordWrap(true);
    detect_info->hide();
    sec->add_widget(detect_info);

    QString lg = i18n::lang();
    country = new SearchCombo(i18n::t(QStringLiteral("search")), i18n::t(QStringLiteral("no_results")));
    QVector<ComboItem> cItems;
    for (const auto& c : locations::countries()) {
        cItems.append(ComboItem{
            c.name(lg),
            c.code,
            c.code,
            QStringLiteral("%1 %2 %3").arg(c.en, c.ar, c.code).toLower()
        });
    }
    country->set_items(cItems);
    country->set_value(s->country_code);
    connect(country, &SearchCombo::changed, this, &SettingsPage::country_changed);
    sec->add_widget(new Row(i18n::t(QStringLiteral("country")), country, QString(), true));

    region = new SearchCombo(i18n::t(QStringLiteral("search")), i18n::t(QStringLiteral("no_results")));
    fill_regions();
    connect(region, &SearchCombo::changed, this, &SettingsPage::region_changed);
    sec->add_widget(new Row(i18n::t(QStringLiteral("region")), region, QString(), true));
    set_manual_enabled();
    return sec;
}

void SettingsPage::fill_regions() {
    QString lg = i18n::lang();
    auto c = locations::by_code(s->country_code);
    QVector<ComboItem> rItems;
    for (int i = 0; i < c.regions.size(); ++i) {
        const auto& r = c.regions[i];
        rItems.append(ComboItem{
            locations::region_name(r, lg),
            QString(),
            i,
            QStringLiteral("%1 %2").arg(r.en, r.ar).toLower()
        });
    }
    region->set_items(rItems);
    region->set_value(s->region_index);
}

void SettingsPage::set_manual_enabled() {
    country->setEnabled(!s->auto_location);
    region->setEnabled(!s->auto_location);
    detect_btn->setVisible(s->auto_location);
}

void SettingsPage::auto_toggled(bool v) {
    s->auto_location = v;
    save();
    set_manual_enabled();
    if (v) {
        detect();
    }
}

void SettingsPage::detect() {
    detect_btn->set_busy(true);
    detect_btn->setText(i18n::t(QStringLiteral("detecting")));
    locator->detect([this](std::optional<DetectedLocation> res) {
        detect_btn->set_busy(false);
        detect_btn->setText(i18n::t(QStringLiteral("detect_now")));
        if (!res.has_value()) {
            emit state->toast(QStringLiteral("refresh_fail"), QStringLiteral("warn"));
            return;
        }
        const auto& c = res->country;
        int idx = res->region_index;
        const auto& fix = res->fix;
        s->lat = fix.lat;
        s->lng = fix.lng;
        state->set_region(c.code, idx, false);
        s->lat = fix.lat;
        s->lng = fix.lng;
        save();
        country->set_value(c.code);
        fill_regions();
        state->reload();

        const auto& r = c.regions[idx];
        QString lg = i18n::lang();
        QString mark = i18n::is_rtl() ? QStringLiteral("\u200f") : QStringLiteral("\u200e");
        detect_info->setText(QStringLiteral("%1%2 %3 · %4  (%5 %6, %7)")
                                 .arg(mark,
                                      i18n::t(QStringLiteral("detected")),
                                      locations::region_name(r, lg),
                                      c.name(lg),
                                      (fix.source == QStringLiteral("gps")) ? QStringLiteral("GPS") : QStringLiteral("IP"),
                                      QString::number(fix.lat, 'f', 2),
                                      QString::number(fix.lng, 'f', 2)));
        detect_info->show();
    });
}

void SettingsPage::country_changed(const QVariant& codeVar) {
    QString code = codeVar.toString();
    s->country_code = code;
    s->region_index = 0;
    state->set_region(code, 0);
    fill_regions();
}

void SettingsPage::region_changed(const QVariant& idxVar) {
    state->set_region(s->country_code, idxVar.toInt());
}

Section* SettingsPage::make_alerts() {
    auto* sec = new Section(i18n::t(QStringLiteral("sec_alerts")));
    auto* tg = new Toggle(s->notify);
    connect(tg, &Toggle::toggled_by_user, this, [this](bool v) {
        s->notify = v;
        save();
        emit state->alertsChanged();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("enable_alerts")), tg));

    sn_in = new MinutesInput(s->snooze_minutes, 1, 720, i18n::t(QStringLiteral("minutes")));
    connect(sn_in, &MinutesInput::changed, this, [this](int v) {
        s->snooze_minutes = std::max(1, v);
        save();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("snooze_default")), sn_in));

    auto* se = new Toggle(s->sound_enabled);
    connect(se, &Toggle::toggled_by_user, this, [this](bool v) {
        s->sound_enabled = v;
        save();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("sound_enable")), se));

    auto preview_fn = [this](const QString& k) { emit previewSound(k); };
    auto save_fn = [this]() { save(); };
    sec->add_widget(new SoundRow(i18n::t(QStringLiteral("adhan_sound")), s, QStringLiteral("adhan_sound"), QStringLiteral("adhan"), preview_fn, save_fn));
    sec->add_widget(new SoundRow(i18n::t(QStringLiteral("pre_sound")), s, QStringLiteral("pre_sound"), QStringLiteral("pre"), preview_fn, save_fn));
    sec->add_widget(new SoundRow(i18n::t(QStringLiteral("iqama_sound")), s, QStringLiteral("iqama_sound"), QStringLiteral("iqama"), preview_fn, save_fn));
    return sec;
}

QWidget* SettingsPage::make_hdr(const QString& title, const QString& sub) {
    auto* w = new QWidget();
    auto* lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(new Label(title, 10.5, 700, QStringLiteral("accent"), nullptr, Qt::AlignCenter));
    if (!sub.isEmpty()) {
        lay->addWidget(new Label(sub, 8.5, 500, QStringLiteral("text2"), nullptr, Qt::AlignCenter));
    }
    return w;
}

QWidget* SettingsPage::make_cell(Toggle* toggle, MinutesInput* minutes) {
    auto* w = new QWidget();
    auto* lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);
    lay->addWidget(toggle);
    lay->addWidget(minutes);
    return w;
}

Section* SettingsPage::make_prayers() {
    auto* sec = new Section(i18n::t(QStringLiteral("sec_prayers")));
    auto* hint = new Label(i18n::t(QStringLiteral("prayers_hint2")), 10.0, 500, QStringLiteral("text2"));
    hint->setWordWrap(true);
    sec->add_widget(hint);

    auto* g = new QGridLayout();
    g->setHorizontalSpacing(14);
    g->setVerticalSpacing(10);
    g->addWidget(new QWidget(), 0, 0);
    g->addWidget(make_hdr(i18n::t(QStringLiteral("col_alarm"))), 0, 1);
    g->addWidget(make_hdr(i18n::t(QStringLiteral("col_remind")), i18n::t(QStringLiteral("col_before"))), 0, 2);
    g->addWidget(make_hdr(i18n::t(QStringLiteral("iqama")), i18n::t(QStringLiteral("col_after"))), 0, 3);
    g->setColumnStretch(0, 1);

    for (int idx = 0; idx < PRAYER_KEYS.size(); ++idx) {
        int rowIdx = idx + 1;
        QString k = PRAYER_KEYS[idx];
        g->addWidget(new Label(i18n::tp(k), 11.5, 700, QStringLiteral("text")), rowIdx, 0);

        auto* a = new Toggle(s->alerts.value(k, true));
        connect(a, &Toggle::toggled_by_user, this, [this, k](bool v) {
            s->alerts[k] = v;
            save();
            emit state->alertsChanged();
        });
        alarm_t[k] = a;
        g->addWidget(a, rowIdx, 1, Qt::AlignCenter);

        // reminder before the adhan
        auto* rt = new Toggle(s->reminders.value(k, false));
        auto* rm = new MinutesInput(s->reminder_min.value(k, 10), 1, 240, QString(), nullptr, true);
        rm->setEnabled(rt->isChecked());
        connect(rt, &Toggle::toggled_by_user, this, [this, k, rm](bool v) {
            s->reminders[k] = v;
            save();
            rm->setEnabled(v);
        });
        connect(rm, &MinutesInput::changed, this, [this, k](int v) {
            if (v >= 1) {
                s->reminder_min[k] = v;
                save();
            }
        });
        g->addWidget(make_cell(rt, rm), rowIdx, 2, Qt::AlignCenter);

        // iqama after the adhan (not for sunrise)
        if (k == QStringLiteral("sunrise")) {
            g->addWidget(new Label(QStringLiteral("—"), 12.0, 700, QStringLiteral("text3"), nullptr, Qt::AlignCenter), rowIdx, 3);
        } else {
            auto* it = new Toggle(s->iqama.value(k, false));
            auto* im = new MinutesInput(s->iqama_min.value(k, 15), 1, 120, QString(), nullptr, true);
            im->setEnabled(it->isChecked());
            connect(it, &Toggle::toggled_by_user, this, [this, k, im](bool v) {
                s->iqama[k] = v;
                save();
                im->setEnabled(v);
            });
            connect(im, &MinutesInput::changed, this, [this, k](int v) {
                if (v >= 1) {
                    s->iqama_min[k] = v;
                    save();
                }
            });
            g->addWidget(make_cell(it, im), rowIdx, 3, Qt::AlignCenter);
        }
    }
    sec->add_layout(g);
    connect(state, &AppState::alertsChanged, this, &SettingsPage::sync_flags);
    return sec;
}

void SettingsPage::sync_flags() {
    for (auto it = alarm_t.begin(); it != alarm_t.end(); ++it) {
        it.value()->set_value(s->alerts.value(it.key(), true));
    }
}

Section* SettingsPage::make_sources() {
    auto* sec = new Section(i18n::t(QStringLiteral("sec_sources")));
    auto* mode = new Segmented(
        {{QStringLiteral("auto"), i18n::t(QStringLiteral("src_auto"))},
         {QStringLiteral("offline"), i18n::t(QStringLiteral("src_offline"))}},
        s->source_mode
    );
    connect(mode, &Segmented::changed, this, [this](const QString& k) {
        s->source_mode = k;
        save();
        state->reload();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("primary_src")), mode, QString(), true));
    sec->add_widget(new Label(i18n::t(QStringLiteral("priority")), 10.0, 600, QStringLiteral("text2")));

    const std::pair<QString, QString> provs[] = {
        {QStringLiteral("aladhan"), QStringLiteral("1 · Aladhan API")},
        {QStringLiteral("aladhan-city"), QStringLiteral("2 · Aladhan (city)")},
        {QStringLiteral("offline"), QStringLiteral("3 · %1").arg(i18n::t(QStringLiteral("src_local")))}
    };
    for (const auto& [k, name] : provs) {
        auto* b = new StatusBadge(i18n::t(QStringLiteral("idle")), QStringLiteral("idle"));
        badges[k] = b;
        sec->add_widget(new Row(name, b));
    }

    method = new SearchCombo(i18n::t(QStringLiteral("search")), i18n::t(QStringLiteral("no_results")));
    fill_methods();
    connect(method, &SearchCombo::changed, this, [this](const QVariant& kVar) {
        s->method = kVar.toString();
        save();
        state->reload();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("method")), method, QString(), true));

    auto* sc = new Segmented(
        {{QStringLiteral("0"), i18n::t(QStringLiteral("school0"))},
         {QStringLiteral("1"), i18n::t(QStringLiteral("school1"))}},
        QString::number(s->asr_school)
    );
    connect(sc, &Segmented::changed, this, [this](const QString& k) {
        s->asr_school = k.toInt();
        save();
        state->reload();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("school")), sc, QString(), true));

    sec->add_widget(new Label(i18n::t(QStringLiteral("adjust")), 10.0, 600, QStringLiteral("text2")));
    auto* g = new QGridLayout();
    g->setHorizontalSpacing(14);
    g->setVerticalSpacing(10);
    for (int i = 0; i < PRAYER_KEYS.size(); ++i) {
        QString k = PRAYER_KEYS[i];
        auto* st = new Stepper(s->offsets.value(k, 0));
        connect(st, &Stepper::changed, this, [this, k](int v) {
            s->offsets[k] = v;
            save();
            emit state->timesChanged();
        });
        g->addWidget(new Label(i18n::tp(k), 10.5, 600, QStringLiteral("text")), i / 2, (i % 2) * 2);
        g->addWidget(st, i / 2, (i % 2) * 2 + 1);
    }
    sec->add_layout(g);

    refresh_btn = new Button(i18n::t(QStringLiteral("refresh")), QStringLiteral("primary"), QStringLiteral("refresh"), nullptr, 46);
    connect(refresh_btn, &QAbstractButton::clicked, this, [this]() { state->reload(true); });
    sec->add_widget(refresh_btn);
    connect(state, &AppState::busyChanged, refresh_btn, &Button::set_busy);
    return sec;
}

void SettingsPage::fill_methods() {
    QString lg = i18n::lang();
    QVector<ComboItem> items;
    const auto& allM = methods::METHODS();
    auto auto_m = allM.value(methods::default_for(s->country_code));
    QString autoLabel = ((lg == QStringLiteral("ar")) ? QStringLiteral("تلقائي — ") : QStringLiteral("Auto — "))
                        + ((lg == QStringLiteral("ar")) ? auto_m.ar : auto_m.en);
    items.append(ComboItem{autoLabel, QString(), QStringLiteral("auto"), QStringLiteral("auto تلقائي")});
    for (const QString& k : methods::ORDER()) {
        const auto& m = allM.value(k);
        items.append(ComboItem{
            (lg == QStringLiteral("ar")) ? m.ar : m.en,
            QString(),
            k,
            QStringLiteral("%1 %2").arg(m.en, m.ar).toLower()
        });
    }
    method->set_items(items);
    method->set_value(s->method);
}

void SettingsPage::update_status() {
    const auto& st = state->statuses;
    for (auto it = badges.begin(); it != badges.end(); ++it) {
        QString k = it.key();
        QString v = st.value(k, QStringLiteral("idle"));
        if (k == QStringLiteral("offline") && (v == QStringLiteral("ready") || v == QStringLiteral("ok"))) {
            v = QStringLiteral("ready");
        }
        QString txtKey = QStringLiteral("idle");
        QString kind = QStringLiteral("idle");
        if (v == QStringLiteral("ok")) { txtKey = QStringLiteral("connected"); kind = QStringLiteral("ok"); }
        else if (v == QStringLiteral("failed")) { txtKey = QStringLiteral("failed"); kind = QStringLiteral("failed"); }
        else if (v == QStringLiteral("cached")) { txtKey = QStringLiteral("cached"); kind = QStringLiteral("cached"); }
        else if (v == QStringLiteral("ready")) { txtKey = QStringLiteral("offline_ready"); kind = QStringLiteral("ready"); }
        it.value()->set(i18n::t(txtKey), kind);
    }
}

Section* SettingsPage::make_look() {
    auto* sec = new Section(i18n::t(QStringLiteral("sec_look")));
    auto* lg = new Segmented({{QStringLiteral("ar"), QStringLiteral("عربي")}, {QStringLiteral("en"), QStringLiteral("English")}}, i18n::lang());
    connect(lg, &Segmented::changed, this, &SettingsPage::languageChanged);
    sec->add_widget(new Row(i18n::t(QStringLiteral("language")), lg, QString(), true));

    auto* bt = new Button(i18n::t(QStringLiteral("open_themes")), QStringLiteral("ghost"), QStringLiteral("palette"), nullptr, 44);
    connect(bt, &QAbstractButton::clicked, this, &SettingsPage::openThemes);
    sec->add_widget(bt);
    return sec;
}

Section* SettingsPage::make_system() {
    auto* sec = new Section(i18n::t(QStringLiteral("sec_system")));
    if (autostart::supported()) {
        auto* a = new Toggle(s->autostart);
        connect(a, &Toggle::toggled_by_user, this, [this](bool v) {
            s->autostart = v;
            autostart::set_enabled(v);
            save();
        });
        sec->add_widget(new Row(i18n::t(QStringLiteral("autostart")), a));
    }
    auto* tr = new Toggle(s->minimize_to_tray);
    connect(tr, &Toggle::toggled_by_user, this, [this](bool v) {
        s->minimize_to_tray = v;
        save();
    });
    sec->add_widget(new Row(i18n::t(QStringLiteral("tray")), tr));
    return sec;
}

} // namespace vm::ui
