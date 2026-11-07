#pragma once
#include <QColor>
#include <QMap>
#include <QString>
#include <QStringList>
#include <optional>

namespace vm::theme {

struct PaletteEntry {
    QString ar;
    QString en;
    QMap<QString, QString> dark_over;
    QMap<QString, QString> light_over;
};

const QMap<QString, PaletteEntry>& PALETTES();
const QStringList& PALETTE_ORDER();
PaletteEntry gen_palette(const QString& ar, const QString& en, const QString& accent_hex, std::optional<double> hue = std::nullopt);
QMap<QString, QString> build_palette(const QString& name, const QString& mode, const QString& custom_accent = QStringLiteral("#E6B85C"));

class Theme {
public:
    QString mode = QStringLiteral("dark");
    QString palette = QStringLiteral("gold");
    QString custom_accent = QStringLiteral("#E6B85C");

    Theme();
    bool dark() const { return mode == QStringLiteral("dark"); }
    void set_mode(const QString& m, const QString& pal = QStringLiteral("gold"));
    QMap<QString, QColor> preview(const QString& pal, const QString& m) const;
    QColor c(const QString& name, std::optional<double> alpha = std::nullopt) const;
    static QColor on_color(const QColor& bg);
    QString css(const QString& name) const;
    QString qss() const;

private:
    QMap<QString, QString> m_p;
};

Theme& T();

} // namespace vm::theme
