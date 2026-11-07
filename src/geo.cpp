#include "geo.h"
#include "locations.h"
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QDebug>

namespace vm::geo {

static std::optional<QJsonObject> fetch_json_blocking(const QString& urlStr, int timeout_ms = 5000) {
    QNetworkAccessManager nam;
    QNetworkRequest req{QUrl(urlStr)};
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
        return std::nullopt;
    }
    timer.stop();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "ip geo" << urlStr << "failed:" << reply->errorString();
        reply->deleteLater();
        return std::nullopt;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return std::nullopt;
    }
    return doc.object();
}

std::optional<GeoFix> locate_by_ip() {
    // 1) ipwho.is
    if (auto j = fetch_json_blocking(QStringLiteral("https://ipwho.is/"))) {
        if (j->value(QStringLiteral("success")).toBool()) {
            return GeoFix{
                j->value(QStringLiteral("latitude")).toDouble(),
                j->value(QStringLiteral("longitude")).toDouble(),
                j->value(QStringLiteral("country_code")).toString(),
                QStringLiteral("ip")
            };
        }
    }
    // 2) ip-api.com
    if (auto j = fetch_json_blocking(QStringLiteral("http://ip-api.com/json/?fields=status,lat,lon,countryCode"))) {
        if (j->value(QStringLiteral("status")).toString() == QStringLiteral("success")) {
            return GeoFix{
                j->value(QStringLiteral("lat")).toDouble(),
                j->value(QStringLiteral("lon")).toDouble(),
                j->value(QStringLiteral("countryCode")).toString(),
                QStringLiteral("ip")
            };
        }
    }
    // 3) ipapi.co
    if (auto j = fetch_json_blocking(QStringLiteral("https://ipapi.co/json/"))) {
        if (!j->value(QStringLiteral("latitude")).isNull() && !j->value(QStringLiteral("latitude")).isUndefined()) {
            return GeoFix{
                j->value(QStringLiteral("latitude")).toDouble(),
                j->value(QStringLiteral("longitude")).toDouble(),
                j->value(QStringLiteral("country_code")).toString(),
                QStringLiteral("ip")
            };
        }
    }
    return std::nullopt;
}

QString reverse_country(double lat, double lng) {
    return locations::nearest_global(lat, lng).country.code;
}

} // namespace vm::geo
