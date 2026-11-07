#include "providers.h"
#include "methods.h"
#include "paths.h"
#include "prayer_calc.h"
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

namespace vm::providers {

static const QString ALADHAN = QStringLiteral("https://api.aladhan.com/v1");

static QMap<QString, QString> api_key_map() {
    return {
        {QStringLiteral("Fajr"), QStringLiteral("fajr")},
        {QStringLiteral("Sunrise"), QStringLiteral("sunrise")},
        {QStringLiteral("Dhuhr"), QStringLiteral("dhuhr")},
        {QStringLiteral("Asr"), QStringLiteral("asr")},
        {QStringLiteral("Maghrib"), QStringLiteral("maghrib")},
        {QStringLiteral("Isha"), QStringLiteral("isha")}
    };
}

static QJsonObject http_get(const QString& urlStr, const QMap<QString, QString>& params, int tries = 2, int timeout_ms = 6000) {
    QString lastErr;
    for (int i = 0; i < tries; ++i) {
        QUrl url(urlStr);
        QUrlQuery query;
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.addQueryItem(it.key(), it.value());
        }
        url.setQuery(query);

        QNetworkAccessManager nam;
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("V-Mawaqeet/2.0"));
        QNetworkReply* reply = nam.get(req);

        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        timer.start(timeout_ms);
        loop.exec();

        if (!timer.isActive()) {
            reply->abort();
            reply->deleteLater();
            lastErr = QStringLiteral("Timeout");
        } else {
            timer.stop();
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray bytes = reply->readAll();
                reply->deleteLater();
                QJsonDocument doc = QJsonDocument::fromJson(bytes);
                QJsonObject j = doc.object();
                if (j.value(QStringLiteral("code")).toInt() == 200) {
                    return j;
                }
                lastErr = QStringLiteral("API code %1").arg(j.value(QStringLiteral("code")).toInt());
            } else {
                lastErr = reply->errorString();
                reply->deleteLater();
            }
        }
        if (i < tries - 1) {
            QThread::msleep(1000UL * (1UL << i));
        }
    }
    throw std::runtime_error(lastErr.toStdString());
}

static QMap<QString, DayTimes> parse_aladhan(const QJsonObject& payload) {
    QMap<QString, DayTimes> out;
    const auto kmap = api_key_map();
    QJsonArray data = payload.value(QStringLiteral("data")).toArray();
    if (data.isEmpty()) {
        throw std::runtime_error("empty data");
    }
    for (const QJsonValue& itemVal : data) {
        QJsonObject item = itemVal.toObject();
        QString g = item.value(QStringLiteral("date")).toObject()
                        .value(QStringLiteral("gregorian")).toObject()
                        .value(QStringLiteral("date")).toString(); // DD-MM-YYYY
        QStringList parts = g.split(QLatin1Char('-'));
        if (parts.size() != 3) throw std::runtime_error("invalid date");
        QDate d(parts[2].toInt(), parts[1].toInt(), parts[0].toInt());

        QJsonObject timingsObj = item.value(QStringLiteral("timings")).toObject();
        QMap<QString, QString> times;
        for (auto it = kmap.begin(); it != kmap.end(); ++it) {
            if (timingsObj.contains(it.key())) {
                QString raw = timingsObj.value(it.key()).toString();
                QString hhmm = raw.split(QLatin1Char(' ')).first().left(5);
                times[it.value()] = hhmm;
            }
        }
        if (times.size() != 6) {
            throw std::runtime_error("incomplete timings");
        }
        QJsonObject h = item.value(QStringLiteral("date")).toObject()
                            .value(QStringLiteral("hijri")).toObject();
        int hDay = h.value(QStringLiteral("day")).toString().toInt();
        if (hDay == 0) hDay = h.value(QStringLiteral("day")).toInt();
        int hMonth = h.value(QStringLiteral("month")).toObject().value(QStringLiteral("number")).toInt();
        int hYear = h.value(QStringLiteral("year")).toString().toInt();
        if (hYear == 0) hYear = h.value(QStringLiteral("year")).toInt();

        QString iso = d.toString(Qt::ISODate);
        out[iso] = DayTimes{d, times, HijriDate(hDay, hMonth, hYear)};
    }
    return out;
}

static QMap<QString, DayTimes> offline_month(const Place& place, const Settings& s, int year, int month) {
    methods::Method m = methods::resolve(s.method, place.country_code);
    QMap<QString, DayTimes> out;
    QDate d(year, month, 1);
    while (d.isValid() && d.month() == month) {
        auto r = prayer_calc::calculate(d, place.lat, place.lng, place.tz, m, s.asr_school);
        QMap<QString, QString> times;
        for (auto it = r.begin(); it != r.end(); ++it) {
            times[it.key()] = it.value().toString(QStringLiteral("HH:mm"));
        }
        QString iso = d.toString(Qt::ISODate);
        out[iso] = DayTimes{d, times, to_hijri(d)};
        d = d.addDays(1);
    }
    return out;
}

static QString cache_path(const Place& place, int method_id, int school, int year, int month) {
    QString fname = QStringLiteral("%1_%2_m%3_s%4_%5-%6.json")
                        .arg(place.lat, 0, 'f', 2)
                        .arg(place.lng, 0, 'f', 2)
                        .arg(method_id)
                        .arg(school)
                        .arg(year)
                        .arg(month, 2, 10, QLatin1Char('0'));
    return paths::cache_dir().filePath(fname);
}

static std::optional<QMap<QString, DayTimes>> load_cache_file(const QString& path) {
    QFile f(path);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return std::nullopt;
    }
    QJsonObject raw = doc.object();
    QMap<QString, DayTimes> out;
    for (auto it = raw.begin(); it != raw.end(); ++it) {
        QDate d = QDate::fromString(it.key(), Qt::ISODate);
        if (!d.isValid()) return std::nullopt;
        QJsonObject v = it.value().toObject();
        QJsonObject tObj = v.value(QStringLiteral("t")).toObject();
        QMap<QString, QString> times;
        for (auto tit = tObj.begin(); tit != tObj.end(); ++tit) {
            times[tit.key()] = tit.value().toString();
        }
        HijriDate hij;
        if (v.value(QStringLiteral("h")).isArray()) {
            QJsonArray hArr = v.value(QStringLiteral("h")).toArray();
            if (hArr.size() == 3) {
                hij = HijriDate(hArr[0].toInt(), hArr[1].toInt(), hArr[2].toInt());
            }
        }
        out[it.key()] = DayTimes{d, times, hij};
    }
    if (out.isEmpty()) return std::nullopt;
    return out;
}

static void save_cache_file(const QString& path, const QMap<QString, DayTimes>& days) {
    QJsonObject root;
    for (auto it = days.begin(); it != days.end(); ++it) {
        QJsonObject tObj;
        for (auto tit = it.value().times.begin(); tit != it.value().times.end(); ++tit) {
            tObj[tit.key()] = tit.value();
        }
        QJsonObject entry;
        entry[QStringLiteral("t")] = tObj;
        if (it.value().hijri.valid) {
            QJsonArray hArr;
            hArr.append(it.value().hijri.day);
            hArr.append(it.value().hijri.month);
            hArr.append(it.value().hijri.year);
            entry[QStringLiteral("h")] = hArr;
        } else {
            entry[QStringLiteral("h")] = QJsonValue::Null;
        }
        root[it.key()] = entry;
    }
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    } else {
        qWarning() << "cache write failed:" << path;
    }
}

std::optional<MonthResult> load_cached(const Place& place, const Settings& s, int year, int month) {
    if (s.source_mode == QStringLiteral("offline")) {
        return std::nullopt;
    }
    methods::Method m = methods::resolve(s.method, place.country_code);
    auto days = load_cache_file(cache_path(place, m.aladhan_id, s.asr_school, year, month));
    if (days) {
        MonthResult res;
        res.days = *days;
        res.source = QStringLiteral("cache");
        res.statuses = {
            {QStringLiteral("aladhan"), QStringLiteral("cached")},
            {QStringLiteral("offline"), QStringLiteral("ready")}
        };
        return res;
    }
    return std::nullopt;
}

MonthResult local_month(const Place& place, const Settings& s, int year, int month) {
    MonthResult res;
    res.days = offline_month(place, s, year, month);
    res.source = QStringLiteral("offline");
    res.statuses = {
        {QStringLiteral("aladhan"), (s.source_mode == QStringLiteral("offline")) ? QStringLiteral("skipped") : QStringLiteral("idle")},
        {QStringLiteral("offline"), QStringLiteral("ok")}
    };
    return res;
}

MonthResult fetch_month(const Place& place, const Settings& s, int year, int month, bool force) {
    methods::Method m = methods::resolve(s.method, place.country_code);
    if (s.source_mode == QStringLiteral("offline")) {
        return local_month(place, s, year, month);
    }
    QString path = cache_path(place, m.aladhan_id, s.asr_school, year, month);
    if (!force) {
        if (auto cached = load_cache_file(path)) {
            MonthResult res;
            res.days = *cached;
            res.source = QStringLiteral("cache");
            res.statuses = {
                {QStringLiteral("aladhan"), QStringLiteral("cached")},
                {QStringLiteral("offline"), QStringLiteral("ready")}
            };
            return res;
        }
    }

    QMap<QString, QString> statuses = {
        {QStringLiteral("aladhan"), QStringLiteral("idle")},
        {QStringLiteral("aladhan-city"), QStringLiteral("idle")},
        {QStringLiteral("offline"), QStringLiteral("ready")}
    };

    // 1) Aladhan by coordinates
    try {
        QMap<QString, QString> params = {
            {QStringLiteral("method"), QString::number(m.aladhan_id)},
            {QStringLiteral("school"), QString::number(s.asr_school)},
            {QStringLiteral("latitude"), QString::number(place.lat, 'f', 6)},
            {QStringLiteral("longitude"), QString::number(place.lng, 'f', 6)},
            {QStringLiteral("timezonestring"), place.tz}
        };
        QJsonObject j = http_get(QStringLiteral("%1/calendar/%2/%3").arg(ALADHAN).arg(year).arg(month), params);
        auto days = parse_aladhan(j);
        save_cache_file(path, days);
        statuses[QStringLiteral("aladhan")] = QStringLiteral("ok");
        return MonthResult{days, QStringLiteral("aladhan"), statuses};
    } catch (const std::exception& e) {
        qWarning() << "aladhan(coords) failed:" << e.what();
        statuses[QStringLiteral("aladhan")] = QStringLiteral("failed");
    }

    // 2) Aladhan by city name
    try {
        QMap<QString, QString> params = {
            {QStringLiteral("method"), QString::number(m.aladhan_id)},
            {QStringLiteral("school"), QString::number(s.asr_school)},
            {QStringLiteral("city"), place.city},
            {QStringLiteral("country"), place.country}
        };
        QJsonObject j = http_get(QStringLiteral("%1/calendarByCity/%2/%3").arg(ALADHAN).arg(year).arg(month), params);
        auto days = parse_aladhan(j);
        save_cache_file(path, days);
        statuses[QStringLiteral("aladhan-city")] = QStringLiteral("ok");
        return MonthResult{days, QStringLiteral("aladhan-city"), statuses};
    } catch (const std::exception& e) {
        qWarning() << "aladhan(city) failed:" << e.what();
        statuses[QStringLiteral("aladhan-city")] = QStringLiteral("failed");
    }

    // 3) Offline astronomical calculation
    statuses[QStringLiteral("offline")] = QStringLiteral("ok");
    return MonthResult{offline_month(place, s, year, month), QStringLiteral("offline"), statuses};
}

QDateTime now_in(const QString& tz) {
    QTimeZone zone = prayer_calc::resolve_tz(tz);
    return QDateTime::currentDateTimeUtc().toTimeZone(zone);
}

} // namespace vm::providers
