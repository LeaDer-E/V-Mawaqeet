#include "locations.h"
#include "paths.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

namespace vm::locations {

static constexpr double D2R = 3.14159265358979323846 / 180.0;

const QVector<Country>& countries() {
    static QVector<Country> list = []() {
        QVector<Country> out;
        QFile f(paths::asset(QStringLiteral("data"), QStringLiteral("locations.json")));
        if (!f.open(QIODevice::ReadOnly)) {
            Country fallback;
            fallback.code = QStringLiteral("EG");
            fallback.en = QStringLiteral("Egypt");
            fallback.ar = QStringLiteral("مصر");
            fallback.lat = 30.0444;
            fallback.lng = 31.2357;
            fallback.tz = QStringLiteral("Africa/Cairo");
            fallback.regions.append({QStringLiteral("Alexandria"), QStringLiteral("الإسكندرية"), 31.2, 29.92, QStringLiteral("Africa/Cairo")});
            out.append(fallback);
            return out;
        }
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        QJsonArray arr = doc.array();
        out.reserve(arr.size());
        for (const QJsonValue& val : arr) {
            QJsonObject cObj = val.toObject();
            Country c;
            c.code = cObj.value(QStringLiteral("c")).toString();
            c.en = cObj.value(QStringLiteral("en")).toString();
            c.ar = cObj.value(QStringLiteral("ar")).toString();
            c.lat = cObj.value(QStringLiteral("lat")).toDouble(0.0);
            c.lng = cObj.value(QStringLiteral("lng")).toDouble(0.0);
            c.tz = cObj.value(QStringLiteral("tz")).toString(QStringLiteral("UTC"));

            QJsonArray sArr = cObj.value(QStringLiteral("s")).toArray();
            c.regions.reserve(sArr.size());
            for (const QJsonValue& rVal : sArr) {
                QJsonArray rArr = rVal.toArray();
                if (rArr.size() < 5) continue;
                Region r;
                r.en = rArr[0].toString();
                r.ar = rArr[1].toString();
                r.lat = rArr[2].isNull() ? c.lat : rArr[2].toDouble(c.lat);
                r.lng = rArr[3].isNull() ? c.lng : rArr[3].toDouble(c.lng);
                QString rtz = rArr[4].toString();
                r.tz = rtz.isEmpty() ? c.tz : rtz;
                c.regions.append(r);
            }
            if (c.regions.isEmpty()) {
                c.regions.append({c.en, c.ar, c.lat, c.lng, c.tz});
            }
            out.append(c);
        }
        return out;
    }();
    return list;
}

Country by_code(const QString& code) {
    QString up = code.toUpper();
    const auto& all = countries();
    for (const auto& c : all) {
        if (c.code.toUpper() == up) {
            return c;
        }
    }
    return all.first();
}

QString region_name(const Region& r, const QString& lang) {
    return (lang == QStringLiteral("ar")) ? r.ar : r.en;
}

std::pair<QString, QString> label(const Country& c, const Region& r, const QString& lang) {
    return {region_name(r, lang), c.name(lang)};
}

double haversine(double lat1, double lng1, double lat2, double lng2) {
    double p1 = lat1 * D2R;
    double p2 = lat2 * D2R;
    double dp = p2 - p1;
    double dl = (lng2 - lng1) * D2R;
    double s1 = std::sin(dp / 2.0);
    double s2 = std::sin(dl / 2.0);
    double a = s1 * s1 + std::cos(p1) * std::cos(p2) * s2 * s2;
    return 6371.0 * 2.0 * std::asin(std::sqrt(std::max(0.0, std::min(1.0, a))));
}

std::pair<Country, int> nearest(const QString& code, double lat, double lng) {
    Country c = by_code(code);
    int bestIdx = 0;
    double bestDist = 1e18;
    for (int i = 0; i < c.regions.size(); ++i) {
        double d = haversine(lat, lng, c.regions[i].lat, c.regions[i].lng);
        if (d < bestDist) {
            bestDist = d;
            bestIdx = i;
        }
    }
    return {c, bestIdx};
}

NearestGlobalResult nearest_global(double lat, double lng) {
    const auto& all = countries();
    NearestGlobalResult best{all.first(), 0, 1e9};
    for (const auto& c : all) {
        for (int i = 0; i < c.regions.size(); ++i) {
            double d = haversine(lat, lng, c.regions[i].lat, c.regions[i].lng);
            if (d < best.distance_km) {
                best.distance_km = d;
                best.country = c;
                best.region_index = i;
            }
        }
    }
    return best;
}

} // namespace vm::locations
