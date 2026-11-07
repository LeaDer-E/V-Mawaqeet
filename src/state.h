#pragma once
#include "config.h"
#include "geo.h"
#include "hijri.h"
#include "locations.h"
#include "methods.h"
#include "providers.h"
#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimeZone>
#include <QVector>
#include <functional>
#include <optional>
#include <utility>

namespace vm {

struct Snapshot {
    QVector<std::pair<QString, QDateTime>> today;
    QString next_key;
    QDateTime next_dt;
    QDateTime prev_dt;
    QString prev_key;
    QSet<QString> past;
};

struct DetectedLocation {
    locations::Country country;
    int region_index = 0;
    geo::GeoFix fix;
};

class AppState : public QObject {
    Q_OBJECT
public:
    Settings s;
    QMap<QString, providers::DayTimes> days;
    QString source = QStringLiteral("offline");
    QMap<QString, QString> statuses;
    bool busy = false;

    explicit AppState(const Settings& settings, QObject* parent = nullptr);

    locations::Country country() const;
    locations::Region region() const;
    providers::Place place() const;
    void set_region(const QString& code, int index, bool save = true);

    QTimeZone tz() const;
    QDateTime now() const;
    methods::Method method() const;

    QVector<std::pair<QString, QDateTime>> day_prayers(const QDate& d);
    HijriDate hijri(const QDate& d);
    Snapshot snapshot();

    void reload(bool force = false);
    void detect_location(std::function<void(std::optional<DetectedLocation>)> on_done);
    DetectedLocation apply_fix(const geo::GeoFix& fix) const;

signals:
    void timesChanged();
    void busyChanged(bool b);
    void toast(const QString& msg_key, const QString& kind);
    void locationChanged();
    void alertsChanged();

private:
    int m_gen = 0;
    providers::DayTimes ensure_day(const QDate& d);
    QVector<std::pair<int, int>> months_needed() const;
    void set_busy(bool b);
};

} // namespace vm
