#pragma once
#include <QMap>
#include <QString>
#include <QStringList>

namespace vm::methods {

struct Method {
    QString key;
    int aladhan_id = 3;
    QString ar;
    QString en;
    double fajr = 18.0;
    double isha = 17.0;
    int isha_min = 0;
    double maghrib_min = 0.0;
};

const QMap<QString, Method>& METHODS();
const QStringList& ORDER();
QString default_for(const QString& country_code);
Method resolve(const QString& key, const QString& country_code);

} // namespace vm::methods
