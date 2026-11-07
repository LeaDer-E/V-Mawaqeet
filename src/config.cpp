#include "config.h"
#include "paths.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace vm {

static QMap<QString, int> default_iqama_min() {
    return {
        {QStringLiteral("fajr"), 20},
        {QStringLiteral("sunrise"), 0},
        {QStringLiteral("dhuhr"), 15},
        {QStringLiteral("asr"), 15},
        {QStringLiteral("maghrib"), 10},
        {QStringLiteral("isha"), 15}
    };
}

Settings::Settings() {
    const auto iqDef = default_iqama_min();
    for (const QString& k : PRAYER_KEYS) {
        offsets[k] = 0;
        alerts[k] = (k != QStringLiteral("sunrise"));
        reminders[k] = false;
        reminder_min[k] = 10;
        iqama[k] = false;
        iqama_min[k] = iqDef.value(k, 15);
    }
}

Settings Settings::load() {
    QString p = paths::user_dir().filePath(QStringLiteral("config.json"));
    QFile f(p);
    if (!f.exists()) {
        return Settings();
    }
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "config unreadable, using defaults";
        return Settings();
    }
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "config JSON invalid, using defaults:" << err.errorString();
        return Settings();
    }

    QJsonObject raw = doc.object();
    Settings s;

    // Migrate v2.0 settings
    if (!raw.value(QStringLiteral("custom_sound")).toString().isEmpty() &&
        raw.value(QStringLiteral("adhan_sound")).toString().isEmpty()) {
        raw[QStringLiteral("adhan_sound")] = raw.value(QStringLiteral("custom_sound"));
    }
    if (raw.value(QStringLiteral("sound_mode")).toString() == QStringLiteral("silent")) {
        raw[QStringLiteral("sound_enabled")] = false;
    }

    if (raw.contains(QStringLiteral("first_run"))) s.first_run = raw.value(QStringLiteral("first_run")).toBool(true);
    if (raw.contains(QStringLiteral("language"))) s.language = raw.value(QStringLiteral("language")).toString(QStringLiteral("ar"));
    if (raw.contains(QStringLiteral("theme"))) s.theme = raw.value(QStringLiteral("theme")).toString(QStringLiteral("dark"));
    if (raw.contains(QStringLiteral("auto_location"))) s.auto_location = raw.value(QStringLiteral("auto_location")).toBool(false);
    if (raw.contains(QStringLiteral("country_code"))) s.country_code = raw.value(QStringLiteral("country_code")).toString(QStringLiteral("EG"));
    if (raw.contains(QStringLiteral("region_index"))) s.region_index = raw.value(QStringLiteral("region_index")).toInt(0);
    if (raw.contains(QStringLiteral("lat"))) s.lat = raw.value(QStringLiteral("lat")).toDouble(31.2);
    if (raw.contains(QStringLiteral("lng"))) s.lng = raw.value(QStringLiteral("lng")).toDouble(29.92);
    if (raw.contains(QStringLiteral("tz"))) s.tz = raw.value(QStringLiteral("tz")).toString(QStringLiteral("Africa/Cairo"));
    if (raw.contains(QStringLiteral("method"))) s.method = raw.value(QStringLiteral("method")).toString(QStringLiteral("auto"));
    if (raw.contains(QStringLiteral("asr_school"))) s.asr_school = raw.value(QStringLiteral("asr_school")).toInt(0);
    if (raw.contains(QStringLiteral("source_mode"))) s.source_mode = raw.value(QStringLiteral("source_mode")).toString(QStringLiteral("auto"));
    if (raw.contains(QStringLiteral("notify"))) s.notify = raw.value(QStringLiteral("notify")).toBool(true);
    if (raw.contains(QStringLiteral("notify_sunrise"))) s.notify_sunrise = raw.value(QStringLiteral("notify_sunrise")).toBool(false);
    if (raw.contains(QStringLiteral("iqama_sound"))) s.iqama_sound = raw.value(QStringLiteral("iqama_sound")).toString();
    if (raw.contains(QStringLiteral("snooze_minutes"))) s.snooze_minutes = raw.value(QStringLiteral("snooze_minutes")).toInt(5);
    if (raw.contains(QStringLiteral("pre_reminder"))) s.pre_reminder = raw.value(QStringLiteral("pre_reminder")).toInt(0);
    if (raw.contains(QStringLiteral("sound_enabled"))) s.sound_enabled = raw.value(QStringLiteral("sound_enabled")).toBool(true);
    if (raw.contains(QStringLiteral("adhan_sound"))) s.adhan_sound = raw.value(QStringLiteral("adhan_sound")).toString();
    if (raw.contains(QStringLiteral("pre_sound"))) s.pre_sound = raw.value(QStringLiteral("pre_sound")).toString();
    if (raw.contains(QStringLiteral("palette"))) s.palette = raw.value(QStringLiteral("palette")).toString(QStringLiteral("gold"));
    if (raw.contains(QStringLiteral("custom_accent"))) s.custom_accent = raw.value(QStringLiteral("custom_accent")).toString(QStringLiteral("#E6B85C"));
    if (raw.contains(QStringLiteral("autostart"))) s.autostart = raw.value(QStringLiteral("autostart")).toBool(false);
    if (raw.contains(QStringLiteral("minimize_to_tray"))) s.minimize_to_tray = raw.value(QStringLiteral("minimize_to_tray")).toBool(true);

    auto readIntMap = [&](const QString& key, QMap<QString, int>& target) {
        if (raw.contains(key) && raw.value(key).isObject()) {
            QJsonObject obj = raw.value(key).toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                target[it.key()] = it.value().toInt();
            }
        }
    };
    auto readBoolMap = [&](const QString& key, QMap<QString, bool>& target) {
        if (raw.contains(key) && raw.value(key).isObject()) {
            QJsonObject obj = raw.value(key).toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                target[it.key()] = it.value().toBool();
            }
        }
    };

    readIntMap(QStringLiteral("offsets"), s.offsets);
    readBoolMap(QStringLiteral("alerts"), s.alerts);
    readBoolMap(QStringLiteral("reminders"), s.reminders);
    readIntMap(QStringLiteral("reminder_min"), s.reminder_min);
    readBoolMap(QStringLiteral("iqama"), s.iqama);
    readIntMap(QStringLiteral("iqama_min"), s.iqama_min);

    if (!raw.contains(QStringLiteral("alerts")) && raw.value(QStringLiteral("notify_sunrise")).toBool(false)) {
        s.alerts[QStringLiteral("sunrise")] = true;
    }
    if (!raw.contains(QStringLiteral("reminder_min"))) {
        int pre = raw.value(QStringLiteral("pre_reminder")).toInt(0);
        for (const QString& k : PRAYER_KEYS) {
            s.reminder_min[k] = (pre > 0 ? pre : 10);
        }
        if (!raw.contains(QStringLiteral("reminders")) || pre == 0) {
            for (const QString& k : PRAYER_KEYS) {
                s.reminders[k] = (pre > 0 && k != QStringLiteral("sunrise"));
            }
        }
    }

    const auto iqDef = default_iqama_min();
    for (const QString& k : PRAYER_KEYS) {
        if (!s.offsets.contains(k)) s.offsets[k] = 0;
        if (!s.alerts.contains(k)) s.alerts[k] = (k != QStringLiteral("sunrise"));
        if (!s.reminders.contains(k)) s.reminders[k] = false;
        if (!s.reminder_min.contains(k)) s.reminder_min[k] = 10;
        if (!s.iqama.contains(k)) s.iqama[k] = false;
        if (!s.iqama_min.contains(k)) s.iqama_min[k] = iqDef.value(k, 15);
    }

    return s;
}

void Settings::save() const {
    QJsonObject root;
    root[QStringLiteral("first_run")] = first_run;
    root[QStringLiteral("language")] = language;
    root[QStringLiteral("theme")] = theme;
    root[QStringLiteral("auto_location")] = auto_location;
    root[QStringLiteral("country_code")] = country_code;
    root[QStringLiteral("region_index")] = region_index;
    root[QStringLiteral("lat")] = lat;
    root[QStringLiteral("lng")] = lng;
    root[QStringLiteral("tz")] = tz;
    root[QStringLiteral("method")] = method;
    root[QStringLiteral("asr_school")] = asr_school;

    auto toJsonIntObj = [](const QMap<QString, int>& m) {
        QJsonObject o;
        for (auto it = m.begin(); it != m.end(); ++it) o[it.key()] = it.value();
        return o;
    };
    auto toJsonBoolObj = [](const QMap<QString, bool>& m) {
        QJsonObject o;
        for (auto it = m.begin(); it != m.end(); ++it) o[it.key()] = it.value();
        return o;
    };

    root[QStringLiteral("offsets")] = toJsonIntObj(offsets);
    root[QStringLiteral("source_mode")] = source_mode;
    root[QStringLiteral("notify")] = notify;
    root[QStringLiteral("notify_sunrise")] = notify_sunrise;
    root[QStringLiteral("alerts")] = toJsonBoolObj(alerts);
    root[QStringLiteral("reminders")] = toJsonBoolObj(reminders);
    root[QStringLiteral("reminder_min")] = toJsonIntObj(reminder_min);
    root[QStringLiteral("iqama")] = toJsonBoolObj(iqama);
    root[QStringLiteral("iqama_min")] = toJsonIntObj(iqama_min);
    root[QStringLiteral("iqama_sound")] = iqama_sound;
    root[QStringLiteral("snooze_minutes")] = snooze_minutes;
    root[QStringLiteral("pre_reminder")] = pre_reminder;
    root[QStringLiteral("sound_enabled")] = sound_enabled;
    root[QStringLiteral("adhan_sound")] = adhan_sound;
    root[QStringLiteral("pre_sound")] = pre_sound;
    root[QStringLiteral("palette")] = palette;
    root[QStringLiteral("custom_accent")] = custom_accent;
    root[QStringLiteral("autostart")] = autostart;
    root[QStringLiteral("minimize_to_tray")] = minimize_to_tray;

    QString p = paths::user_dir().filePath(QStringLiteral("config.json"));
    QFile f(p);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    } else {
        qWarning() << "could not save config";
    }
}

} // namespace vm
