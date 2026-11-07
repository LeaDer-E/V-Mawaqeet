#pragma once
#include "../state.h"
#include <QObject>
#include <QTimer>
#include <functional>
#include <optional>

namespace vm::ui {

class Locator : public QObject {
    Q_OBJECT
public:
    AppState* state = nullptr;
    explicit Locator(AppState* st, QObject* parent = nullptr);

    void detect(std::function<void(std::optional<DetectedLocation>)> cb);

private slots:
    void fallback();

private:
    void finish(std::optional<DetectedLocation> res);
    std::function<void(std::optional<DetectedLocation>)> m_cb;
    bool m_done = true;
    QTimer* m_timeout = nullptr;
};

} // namespace vm::ui
