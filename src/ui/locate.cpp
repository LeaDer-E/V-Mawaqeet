#include "locate.h"
#include "../geo.h"

#if defined(VM_HAS_POSITIONING)
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#endif

namespace vm::ui {

Locator::Locator(AppState* st, QObject* parent)
    : QObject(parent), state(st) {
    m_timeout = new QTimer(this);
    m_timeout->setSingleShot(true);
    connect(m_timeout, &QTimer::timeout, this, &Locator::fallback);
}

void Locator::detect(std::function<void(std::optional<DetectedLocation>)> cb) {
    m_cb = std::move(cb);
    m_done = false;
#if defined(VM_HAS_POSITIONING)
    if (auto* src = QGeoPositionInfoSource::createDefaultSource(this)) {
        connect(src, &QGeoPositionInfoSource::positionUpdated, this, [this](const QGeoPositionInfo& info) {
            if (m_done) return;
            auto c = info.coordinate();
            if (!c.isValid()) {
                fallback();
                return;
            }
            finish(state->apply_fix(geo::GeoFix{
                c.latitude(),
                c.longitude(),
                geo::reverse_country(c.latitude(), c.longitude()),
                QStringLiteral("gps")
            }));
        });
        connect(src, &QGeoPositionInfoSource::errorOccurred, this, [this](auto) {
            fallback();
        });
        src->requestUpdate(6000);
        m_timeout->start(7000);
        return;
    }
#endif
    fallback();
}

void Locator::fallback() {
    if (m_done) return;
    m_timeout->stop();
    m_done = true;
    state->detect_location([this](std::optional<DetectedLocation> res) {
        if (m_cb) m_cb(res);
    });
}

void Locator::finish(std::optional<DetectedLocation> res) {
    m_done = true;
    m_timeout->stop();
    if (m_cb) m_cb(res);
}

} // namespace vm::ui
