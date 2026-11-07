#include "notifier.h"

namespace vm {

Notifier::Notifier(AppState* st, QObject* parent)
    : QObject(parent), state(st) {
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &Notifier::tick);
    m_timer->start();
}

void Notifier::snooze(const QString& key, int minutes) {
    snoozes[key] = state->now().addSecs(static_cast<qint64>(minutes) * 60);
    emit snoozeChanged();
}

void Notifier::cancel_snooze() {
    snoozes.clear();
    emit snoozeChanged();
}

std::optional<std::pair<QString, QDateTime>> Notifier::next_snooze() const {
    if (snoozes.isEmpty()) {
        return std::nullopt;
    }
    auto bestIt = snoozes.begin();
    for (auto it = snoozes.begin(); it != snoozes.end(); ++it) {
        if (it.value() < bestIt.value()) {
            bestIt = it;
        }
    }
    return std::make_pair(bestIt.key(), bestIt.value());
}

bool Notifier::due(const QDateTime& now, const QDateTime& when) {
    qint64 diff = when.secsTo(now);
    return diff >= 0 && diff < 90;
}

void Notifier::tick() {
    const auto& s = state->s;
    if (!s.notify) {
        return;
    }
    QDateTime now = state->now();

    // Check active snoozes
    const QStringList skeys = snoozes.keys();
    for (const QString& key : skeys) {
        QDateTime until = snoozes.value(key);
        if (now >= until) {
            snoozes.remove(key);
            emit snoozeChanged();
            emit fire(key, QStringLiteral("snooze"), until);
        }
    }

    auto prayers = state->day_prayers(now.date());
    for (const auto& [key, dt] : prayers) {
        QString dIso = dt.date().toString(Qt::ISODate);

        // 1) Adhan alarm
        QString adhanToken = QStringLiteral("%1|%2|adhan").arg(dIso, key);
        if (s.alerts.value(key, key != QStringLiteral("sunrise")) &&
            due(now, dt) &&
            !m_fired.contains(adhanToken)) {
            m_fired.insert(adhanToken);
            emit fire(key, QStringLiteral("adhan"), dt);
        }

        // 2) Pre-reminder N minutes before adhan
        int pm = s.reminder_min.value(key, 10);
        QString preToken = QStringLiteral("%1|%2|pre").arg(dIso, key);
        if (s.reminders.value(key, false) &&
            pm > 0 &&
            due(now, dt.addSecs(-static_cast<qint64>(pm) * 60)) &&
            !m_fired.contains(preToken)) {
            m_fired.insert(preToken);
            emit fire(key, QStringLiteral("pre"), dt);
        }

        // 3) Iqama N minutes after adhan (not for sunrise)
        int im = s.iqama_min.value(key, 15);
        QString iqamaToken = QStringLiteral("%1|%2|iqama").arg(dIso, key);
        if (key != QStringLiteral("sunrise") &&
            s.iqama.value(key, false) &&
            im > 0 &&
            due(now, dt.addSecs(static_cast<qint64>(im) * 60)) &&
            !m_fired.contains(iqamaToken)) {
            m_fired.insert(iqamaToken);
            emit fire(key, QStringLiteral("iqama"), dt);
        }
    }

    if (m_fired.size() > 200) {
        QString todayIso = now.date().toString(Qt::ISODate);
        QSet<QString> pruned;
        for (const QString& f : m_fired) {
            if (f.section(QLatin1Char('|'), 0, 0) >= todayIso) {
                pruned.insert(f);
            }
        }
        m_fired = pruned;
    }
}

} // namespace vm
