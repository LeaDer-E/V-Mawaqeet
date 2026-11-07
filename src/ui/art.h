#pragma once
#include <QColor>
#include <QPixmap>
#include <QString>
#include <optional>
#include <utility>

namespace vm::ui::art {

std::pair<QColor, QColor> hero_colors(const QString& key);
QPixmap render(
    const QString& key,
    int w,
    int h,
    double dpr = 2.0,
    std::optional<double> unit = std::nullopt,
    std::optional<double> cx = std::nullopt
);

} // namespace vm::ui::art
