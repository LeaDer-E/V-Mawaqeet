#pragma once
#include "state.h"
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>
#include <optional>
#include <utility>

namespace vm {

class Notifier : public QObject {
    Q_OBJECT
public:
    AppState* state = nullptr;
    QMap<QString, QDateTime> snoozes;

    explicit Notifier(AppState* st, QObject* parent = nullptr);

    void snooze(const QString& key, int minutes);
    void cancel_snooze();
    std::optional<std::pair<QString, QDateTime>> next_snooze() const;

public slots:
    void tick();

signals:
    void fire(const QString& key, const QString& kind, const QDateTime& dt);
    void snoozeChanged();

private:
    QSet<QString> m_fired;
    QTimer* m_timer = nullptr;
    static bool due(const QDateTime& now, const QDateTime& when);
};

} // namespace vm
