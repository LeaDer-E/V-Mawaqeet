#pragma once
#include <QString>
#include <QVector>
#include <utility>

namespace vm::locations {

struct Region {
    QString en;
    QString ar;
    double lat = 0.0;
    double lng = 0.0;
    QString tz;
};

struct Country {
    QString code;
    QString en;
    QString ar;
    double lat = 0.0;
    double lng = 0.0;
    QString tz;
    QVector<Region> regions;

    QString name(const QString& lang) const {
        return (lang == QStringLiteral("ar")) ? ar : en;
    }
};

const QVector<Country>& countries();
Country by_code(const QString& code);
QString region_name(const Region& r, const QString& lang);
std::pair<QString, QString> label(const Country& c, const Region& r, const QString& lang);
double haversine(double lat1, double lng1, double lat2, double lng2);
std::pair<Country, int> nearest(const QString& code, double lat, double lng);

struct NearestGlobalResult {
    Country country;
    int region_index = 0;
    double distance_km = 0.0;
};
NearestGlobalResult nearest_global(double lat, double lng);

} // namespace vm::locations
