#include "prayer_calc.h"
#include <QTime>
#include <cmath>
#include <limits>

namespace vm::prayer_calc {

static constexpr double D2R = 3.14159265358979323846 / 180.0;
static constexpr double R2D = 180.0 / 3.14159265358979323846;

static inline double dsin(double d) { return std::sin(d * D2R); }
static inline double dcos(double d) { return std::cos(d * D2R); }
static inline double dtan(double d) { return std::tan(d * D2R); }
static inline double dasin(double x) { return std::asin(x) * R2D; }
static inline double dacos(double x) { return std::acos(x) * R2D; }
static inline double datan2(double y, double x) { return std::atan2(y, x) * R2D; }
static inline double dacot(double x) { return std::atan(1.0 / x) * R2D; }
static inline double dfix(double a, double m) { return a - m * std::floor(a / m); }

static double julian(int y, int m, int d) {
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    double a = std::floor(y / 100.0);
    double b = 2.0 - a + std::floor(a / 4.0);
    return std::floor(365.25 * (y + 4716)) + std::floor(30.6001 * (m + 1)) + d + b - 1524.5;
}

static std::pair<double, double> sun_pos(double jd) {
    double D = jd - 2451545.0;
    double g = dfix(357.529 + 0.98560028 * D, 360.0);
    double q = dfix(280.459 + 0.98564736 * D, 360.0);
    double L = dfix(q + 1.915 * dsin(g) + 0.020 * dsin(2.0 * g), 360.0);
    double e = 23.439 - 0.00000036 * D;
    double ra = datan2(dcos(e) * dsin(L), dcos(L)) / 15.0;
    double eqt = q / 15.0 - dfix(ra, 24.0);
    return {dasin(dsin(e) * dsin(L)), eqt};
}

static double mid_day(double jd, double t) {
    return dfix(12.0 - sun_pos(jd + t).second, 24.0);
}

static double angle_time(double jd, double lat, double angle, double t, bool ccw = false) {
    double decl = sun_pos(jd + t).first;
    double noon = mid_day(jd, t);
    double x = (-dsin(angle) - dsin(decl) * dsin(lat)) / (dcos(decl) * dcos(lat));
    if (x < -1.0 || x > 1.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    double v = dacos(x) / 15.0;
    return noon + (ccw ? -v : v);
}

static double asr_time(double jd, double lat, double factor, double t) {
    double decl = sun_pos(jd + t).first;
    double angle = -dacot(factor + dtan(std::abs(lat - decl)));
    return angle_time(jd, lat, angle, t, false);
}

QTimeZone resolve_tz(const QString& tz_id) {
    QTimeZone zone(tz_id.toUtf8());
    if (zone.isValid()) {
        return zone;
    }
    QTimeZone sys = QTimeZone::systemTimeZone();
    return sys.isValid() ? sys : QTimeZone::utc();
}

QMap<QString, QDateTime> calculate(
    const QDate& d,
    double lat,
    double lng,
    const QString& tz,
    const methods::Method& method,
    int school
) {
    QTimeZone zone = resolve_tz(tz);
    QDateTime noon_local(d, QTime(12, 0), zone);
    double off = noon_local.offsetFromUtc() / 3600.0;
    double jd = julian(d.year(), d.month(), d.day()) - lng / (15.0 * 24.0);

    QMap<QString, double> t = {
        {QStringLiteral("fajr"), 5.0 / 24.0},
        {QStringLiteral("sunrise"), 6.0 / 24.0},
        {QStringLiteral("dhuhr"), 12.0 / 24.0},
        {QStringLiteral("asr"), 13.0 / 24.0},
        {QStringLiteral("maghrib"), 18.0 / 24.0},
        {QStringLiteral("isha"), 18.0 / 24.0}
    };

    QMap<QString, double> r;
    for (int iter = 0; iter < 2; ++iter) {
        r[QStringLiteral("fajr")] = angle_time(jd, lat, method.fajr, t[QStringLiteral("fajr")], true);
        r[QStringLiteral("sunrise")] = angle_time(jd, lat, 0.833, t[QStringLiteral("sunrise")], true);
        r[QStringLiteral("dhuhr")] = mid_day(jd, t[QStringLiteral("dhuhr")]);
        r[QStringLiteral("asr")] = asr_time(jd, lat, 1.0 + (school == 1 ? 1.0 : 0.0), t[QStringLiteral("asr")]);
        double mag_ang = (method.maghrib_min > 0.0) ? (0.833 + method.maghrib_min) : 0.833;
        r[QStringLiteral("maghrib")] = angle_time(jd, lat, mag_ang, t[QStringLiteral("maghrib")], false);

        if (method.isha_min > 0) {
            r[QStringLiteral("isha")] = r[QStringLiteral("maghrib")] + method.isha_min / 60.0;
        } else {
            r[QStringLiteral("isha")] = angle_time(jd, lat, method.isha, t[QStringLiteral("isha")], false);
        }

        double sr = r[QStringLiteral("sunrise")];
        double ss = r[QStringLiteral("maghrib")];
        if (!std::isnan(sr) && !std::isnan(ss)) {
            double night = dfix(sr - ss, 24.0);
            if (std::isnan(r[QStringLiteral("fajr")])) {
                r[QStringLiteral("fajr")] = sr - night / 7.0;
            }
            if (std::isnan(r[QStringLiteral("isha")])) {
                r[QStringLiteral("isha")] = ss + night / 7.0;
            }
        }
        for (auto it = r.begin(); it != r.end(); ++it) {
            t[it.key()] = (std::isnan(it.value()) ? 12.0 : it.value()) / 24.0;
        }
    }

    double shift = off - lng / 15.0;
    QMap<QString, QDateTime> out;
    QDateTime base(d, QTime(0, 0), zone);
    for (auto it = r.begin(); it != r.end(); ++it) {
        double v = std::isnan(it.value()) ? 12.0 : it.value();
        v += shift;
        qint64 sec = static_cast<qint64>(std::llround(v * 60.0)) * 60; // round to nearest minute
        out[it.key()] = base.addSecs(sec);
    }
    return out;
}

} // namespace vm::prayer_calc
