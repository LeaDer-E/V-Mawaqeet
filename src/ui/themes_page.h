#pragma once
#include "../config.h"
#include "widgets.h"
#include <QAbstractButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>

namespace vm::ui {

class PaletteCard : public QAbstractButton {
    Q_OBJECT
public:
    static constexpr int W = 232;
    static constexpr int H = 196;

    QString name;
    bool sel = false;

    PaletteCard(const QString& name_, bool selected, QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void go(double v);
    double m_h = 0.0;
    QVariantAnimation* m_a = nullptr;
};

class ThemesPage : public QWidget {
    Q_OBJECT
public:
    Settings* s = nullptr;
    Segmented* mode = nullptr;
    QScrollArea* scroll = nullptr;
    QWidget* host = nullptr;
    QGridLayout* grid = nullptr;
    QVector<PaletteCard*> cards;

    explicit ThemesPage(Settings* settings, QWidget* parent = nullptr);

signals:
    void modeChanged(const QString& mode);
    void paletteChanged(const QString& name);
    void customChosen(const QString& hex_color);

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    void pick(const QString& name);
    void flow();
    int m_cols = 0;
};

} // namespace vm::ui
