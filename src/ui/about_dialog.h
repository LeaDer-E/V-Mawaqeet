#pragma once
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QString>
#include <QWidget>
#include <functional>

namespace vm::ui {

class AboutDialog : public QWidget {
    Q_OBJECT
public:
    QString scene_key;
    std::function<void(const QString&)> toast_cb;

    AboutDialog(QWidget* parent, const QString& scene_key_, std::function<void(const QString&)> toast_cb_);

    void open();
    void close_animated();

signals:
    void closed();

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private slots:
    void on_done();

private:
    void build();
    void copy_text(const QString& text);

    QWidget* m_card = nullptr;
    QGraphicsOpacityEffect* m_fx = nullptr;
    QPropertyAnimation* m_a = nullptr;
    QPropertyAnimation* m_b = nullptr;
};

} // namespace vm::ui
