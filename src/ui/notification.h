#pragma once
#include <QLineEdit>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace vm::ui {

class NotificationPopup : public QWidget {
    Q_OBJECT
public:
    QString key = QStringLiteral("fajr");
    QVBoxLayout* root = nullptr;
    QLineEdit* minutes = nullptr;

    explicit NotificationPopup(QWidget* parent = nullptr);

    void present(
        const QString& key_,
        const QString& time_text,
        const QString& kind,
        int pre_minutes = 0,
        int snooze_default = 5
    );
    void dismiss();
    void stop();

signals:
    void stopped();
    void snoozed(const QString& key, int minutes);
    void openApp();

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void clear_layout(QLayout* lay);
    void build(const QString& title, const QString& name, const QString& sub, int snooze_default, bool with_snooze = true);
    void do_snooze(int m);
    void do_snooze_custom();
    void do_open();

    QPixmap m_bg;
    QTimer* m_auto = nullptr;
    QParallelAnimationGroup* m_grp = nullptr;
};

} // namespace vm::ui
