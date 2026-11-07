#include "state.h"
#include "prayer_calc.h"
#include "workers.h"
#include <QTime>
#include <algorithm>

namespace vm {

AppState::AppState(const Settings& settings, QObject* parent)
    : QObject(parent), s(settings) {
    statuses = {
        {QStringLiteral("aladhan"), QStringLiteral("idle")},
        {QStringLiteral("offline"), QStringLiteral("ready")}
    };
}

locations::Country AppState::country() const {
    return locations::by_code(s.country_code);
}

locations::Region AppState::region() const {
    auto regs = country().regions;
    int idx = std::min(std::max(s.region_index, 0), static_cast<int>(regs.size()) - 1);
    return regs[idx];
}

providers::Place AppState::place() const {
    auto c = country();
    auto r = region();
    return providers::Place{
        s.lat,
        s.lng,
        s.tz.isEmpty() ? r.tz : s.tz,
        r.en,
        c.en,
        c.code
    };
}

void AppState::set_region(const QString& code, int index, bool save) {
    auto c = locations::by_code(code);
    index = std::min(std::max(index, 0), static_cast<int>(c.regions.size()) - 1);
    const auto& r = c.regions[index];
    s.country_code = c.code;
    s.region_index = index;
    s.lat = r.lat;
    s.lng = r.lng;
    s.tz = r.tz;
    if (save) {
        s.save();
    }
    emit locationChanged();
    reload();
}

QTimeZone AppState::tz() const {
    return prayer_calc::resolve_tz(s.tz);
}

QDateTime AppState::now() const {
    return providers::now_in(s.tz);
}

methods::Method AppState::method() const {
    return methods::resolve(s.method, s.country_code);
}

providers::DayTimes AppState::ensure_day(const QDate& d) {
    QString iso = d.toString(Qt::ISODate);
    auto it = days.find(iso);
    if (it == days.end()) {
        auto m = method();
        auto r = prayer_calc::calculate(d, s.lat, s.lng, s.tz, m, s.asr_school);
        QMap<QString, QString> tmap;
        for (auto rit = r.begin(); rit != r.end(); ++rit) {
            tmap[rit.key()] = rit.value().toString(QStringLiteral("HH:mm"));
        }
        providers::DayTimes dt{d, tmap, to_hijri(d)};
        days.insert(iso, dt);
        return dt;
    }
    return *it;
}

QVector<std::pair<QString, QDateTime>> AppState::day_prayers(const QDate& d) {
    auto dt = ensure_day(d);
    QTimeZone zone = tz();
    QVector<std::pair<QString, QDateTime>> out;
    out.reserve(PRAYER_KEYS.size());
    for (const QString& k : PRAYER_KEYS) {
        QStringList parts = dt.times.value(k, QStringLiteral("12:00")).split(QLatin1Char(':'));
        int h = (parts.size() > 0) ? parts[0].toInt() : 12;
        int m = (parts.size() > 1) ? parts[1].toInt() : 0;
        QDateTime t(d, QTime(h, m), zone);
        t = t.addSecs(static_cast<qint64>(s.offsets.value(k, 0)) * 60);
        out.append({k, t});
    }
    return out;
}

HijriDate AppState::hijri(const QDate& d) {
    auto dt = ensure_day(d);
    return dt.hijri.valid ? dt.hijri : to_hijri(d);
}

Snapshot AppState::snapshot() {
    QDateTime cur = now();
    QDate todayDate = cur.date();
    auto today = day_prayers(todayDate);
    auto prev_list = day_prayers(todayDate.addDays(-1));
    auto next_list = day_prayers(todayDate.addDays(1));

    QVector<std::pair<QString, QDateTime>> allp;
    allp.reserve(prev_list.size() + today.size() + next_list.size());
    allp.append(prev_list);
    allp.append(today);
    allp.append(next_list);

    std::pair<QString, QDateTime> nxt = allp.last();
    for (const auto& item : allp) {
        if (item.second > cur) {
            nxt = item;
            break;
        }
    }

    QDateTime prevDt = nxt.second.addSecs(-6 * 3600);
    QString prevKey = QStringLiteral("isha");
    for (const auto& item : allp) {
        if (item.second <= cur && item.second >= prevDt) {
            prevDt = item.second;
            prevKey = item.first;
        }
    }

    QSet<QString> past;
    for (const auto& item : today) {
        if (item.second <= cur) {
            past.insert(item.first);
        }
    }

    return Snapshot{today, nxt.first, nxt.second, prevDt, prevKey, past};
}

QVector<std::pair<int, int>> AppState::months_needed() const {
    QDate n = now().date();
    QVector<std::pair<int, int>> ms;
    for (int delta : {-1, 0, 1}) {
        QDate d = n.addDays(delta);
        std::pair<int, int> ym{d.year(), d.month()};
        if (!ms.contains(ym)) {
            ms.append(ym);
        }
    }
    std::sort(ms.begin(), ms.end());
    return ms;
}

void AppState::reload(bool force) {
    m_gen += 1;
    int gen = m_gen;
    providers::Place pl = place();
    days.clear();
    QVector<std::pair<int, int>> missing;

    for (const auto& ym : months_needed()) {
        std::optional<providers::MonthResult> res;
        if (!force) {
            res = providers::load_cached(pl, s, ym.first, ym.second);
        }
        if (!res.has_value()) {
            auto local = providers::local_month(pl, s, ym.first, ym.second);
            if (s.source_mode != QStringLiteral("offline")) {
                missing.append(ym);
            }
            source = QStringLiteral("offline");
            statuses = local.statuses;
            for (auto it = local.days.begin(); it != local.days.end(); ++it) {
                days.insert(it.key(), it.value());
            }
        } else {
            source = res->source;
            statuses = res->statuses;
            for (auto it = res->days.begin(); it != res->days.end(); ++it) {
                days.insert(it.key(), it.value());
            }
        }
    }
    emit timesChanged();

    if (s.source_mode == QStringLiteral("offline")) {
        set_busy(false);
        if (force) {
            emit toast(QStringLiteral("refreshed"), QStringLiteral("ok"));
        }
        return;
    }
    if (missing.isEmpty() && !force) {
        return;
    }
    if (force) {
        missing = months_needed();
    }
    set_busy(true);
    Settings s_copy = s;

    workers::run_async<QVector<providers::MonthResult>>(
        [pl, s_copy, missing, force]() {
            QVector<providers::MonthResult> out;
            for (const auto& ym : missing) {
                out.append(providers::fetch_month(pl, s_copy, ym.first, ym.second, force));
            }
            return out;
        },
        [this, gen, force](std::optional<QVector<providers::MonthResult>> results, QString err) {
            if (gen != m_gen) {
                return;
            }
            set_busy(false);
            if (!err.isEmpty() || !results.has_value() || results->isEmpty()) {
                statuses = {
                    {QStringLiteral("aladhan"), QStringLiteral("failed")},
                    {QStringLiteral("offline"), QStringLiteral("ok")}
                };
                if (force) {
                    emit toast(QStringLiteral("refresh_fail"), QStringLiteral("warn"));
                }
                emit timesChanged();
                return;
            }
            bool online = false;
            for (const auto& r : *results) {
                for (auto it = r.days.begin(); it != r.days.end(); ++it) {
                    days.insert(it.key(), it.value());
                }
                statuses = r.statuses;
                source = r.source;
                online = online || (r.source == QStringLiteral("aladhan") ||
                                    r.source == QStringLiteral("aladhan-city") ||
                                    r.source == QStringLiteral("cache"));
            }
            emit timesChanged();
            if (force || online) {
                emit toast(online ? QStringLiteral("refreshed") : QStringLiteral("refresh_fail"),
                           online ? QStringLiteral("ok") : QStringLiteral("warn"));
            }
        }
    );
}

void AppState::set_busy(bool b) {
    if (busy != b) {
        busy = b;
        emit busyChanged(b);
    }
}

void AppState::detect_location(std::function<void(std::optional<DetectedLocation>)> on_done) {
    workers::run_async<std::optional<geo::GeoFix>>(
        []() { return geo::locate_by_ip(); },
        [this, on_done = std::move(on_done)](std::optional<std::optional<geo::GeoFix>> fixOpt, QString err) {
            if (!err.isEmpty() || !fixOpt.has_value() || !fixOpt->has_value()) {
                on_done(std::nullopt);
                return;
            }
            on_done(apply_fix(**fixOpt));
        }
    );
}

DetectedLocation AppState::apply_fix(const geo::GeoFix& fix) const {
    if (fix.source == QStringLiteral("ip") && !fix.country_code.isEmpty()) {
        auto [c, idx] = locations::nearest(fix.country_code, fix.lat, fix.lng);
        const auto& r = c.regions[idx];
        if (locations::haversine(fix.lat, fix.lng, r.lat, r.lng) < 1200.0) {
            return DetectedLocation{c, idx, fix};
        }
    }
    auto g = locations::nearest_global(fix.lat, fix.lng);
    return DetectedLocation{g.country, g.region_index, fix};
}

} // namespace vm
