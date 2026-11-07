#pragma once
#include "methods.h"
#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QTimeZone>

namespace vm::prayer_calc {

QTimeZone resolve_tz(const QString& tz_id);
QMap<QString, QDateTime> calculate(
    const QDate& d,
    double lat,
    double lng,
    const QString& tz,
    const methods::Method& method,
    int school = 0
);

} // namespace vm::prayer_calc
