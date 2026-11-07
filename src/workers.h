#pragma once
#include <QMetaObject>
#include <QCoreApplication>
#include <QThreadPool>
#include <functional>
#include <optional>
#include <QString>

namespace vm::workers {

template <typename TResult>
void run_async(
    std::function<TResult()> fn,
    std::function<void(std::optional<TResult>, QString)> on_done
) {
    QThreadPool::globalInstance()->start([fn = std::move(fn), on_done = std::move(on_done)]() {
        std::optional<TResult> res;
        QString err;
        try {
            res = fn();
        } catch (const std::exception& e) {
            err = QString::fromUtf8(e.what());
        } catch (...) {
            err = QStringLiteral("Unknown worker error");
        }
        if (QCoreApplication::instance()) {
            QMetaObject::invokeMethod(
                QCoreApplication::instance(),
                [on_done, res = std::move(res), err]() mutable {
                    on_done(std::move(res), err);
                },
                Qt::QueuedConnection
            );
        }
    });
}

} // namespace vm::workers
