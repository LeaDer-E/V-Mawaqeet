#pragma once
#include "config.h"
#include "hijri.h"
#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <optional>

namespace vm::providers {

struct DayTimes {
    QDate day;
    QMap<QString, QString> times; // key -> "HH:MM"
    HijriDate hijri;
};

struct MonthResult {
    QMap<QString, DayTimes> days; // iso date -> DayTimes
    QString source = QStringLiteral("offline"); // aladhan | aladhan-city | cache | offline
    QMap<QString, QString> statuses; // provider -> ok/failed/skipped/cached/ready/idle
};

struct Place {
    double lat = 0.0;
    double lng = 0.0;
    QString tz;
    QString city;
    QString country;
    QString country_code;
};

std::optional<MonthResult> load_cached(const Place& place, const Settings& s, int year, int month);
MonthResult local_month(const Place& place, const Settings& s, int year, int month);
MonthResult fetch_month(const Place& place, const Settings& s, int year, int month, bool force = false);
QDateTime now_in(const QString& tz);

} // namespace vm::providers
