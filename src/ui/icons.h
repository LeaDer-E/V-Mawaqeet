#pragma once
#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QString>
#include <optional>

namespace vm::ui::icons {

void draw(QPainter* p, const QString& name, const QRectF& r, const QColor& color, std::optional<double> w = std::nullopt);
QPixmap pixmap(const QString& name, int size, const QColor& color, double dpr = 2.0, std::optional<double> w = std::nullopt);
QIcon icon(const QString& name, int size, const QColor& color);
QPixmap logo_pixmap(int size, double dpr = 2.0);

} // namespace vm::ui::icons
