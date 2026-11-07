#pragma once
#include <QMap>
#include <QString>
#include <QStringList>

namespace vm {

inline const QStringList PRAYER_KEYS = {
    QStringLiteral("fajr"),
    QStringLiteral("sunrise"),
    QStringLiteral("dhuhr"),
    QStringLiteral("asr"),
    QStringLiteral("maghrib"),
    QStringLiteral("isha")
};

struct Settings {
    bool first_run = true;
    QString language = QStringLiteral("ar");          // ar | en
    QString theme = QStringLiteral("dark");           // dark | light | auto
    // location
    bool auto_location = false;
    QString country_code = QStringLiteral("EG");
    int region_index = 0;                             // index inside the country's region list
    double lat = 31.2;
    double lng = 29.92;
    QString tz = QStringLiteral("Africa/Cairo");
    // calculation
    QString method = QStringLiteral("auto");          // "auto" or a key from methods.METHODS
    int asr_school = 0;                               // 0 = Shafi/Maliki/Hanbali, 1 = Hanafi
    QMap<QString, int> offsets;
    QString source_mode = QStringLiteral("auto");     // auto (online -> offline) | offline
    // notifications
    bool notify = true;
    bool notify_sunrise = false;                      // legacy (v2.0/2.1): migrated into `alerts`
    QMap<QString, bool> alerts;
    QMap<QString, bool> reminders;                    // reminder before the adhan, per prayer
    QMap<QString, int> reminder_min;                  // minutes BEFORE the adhan, per prayer
    QMap<QString, bool> iqama;                        // iqama alert, per prayer
    QMap<QString, int> iqama_min;                     // minutes AFTER the adhan
    QString iqama_sound = QStringLiteral("");         // file path, "" = built-in iqama tone
    int snooze_minutes = 5;
    int pre_reminder = 0;                             // legacy (<= v2.2): migrated into reminders / reminder_min
    bool sound_enabled = true;
    QString adhan_sound = QStringLiteral("");         // file path, "" = built-in chime
    QString pre_sound = QStringLiteral("");           // file path, "" = built-in soft reminder
    QString palette = QStringLiteral("gold");         // key of theme.PALETTES, or "custom"
    QString custom_accent = QStringLiteral("#E6B85C");
    bool autostart = false;
    bool minimize_to_tray = true;

    Settings();
    static Settings load();
    void save() const;
};

} // namespace vm
