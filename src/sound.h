#pragma once
#include "config.h"
#include <QObject>
#include <QString>
#include <QTimer>

#if defined(VM_HAS_MULTIMEDIA)
#include <QAudioOutput>
#include <QMediaPlayer>
#endif

namespace vm {

class SoundPlayer : public QObject {
    Q_OBJECT
public:
    explicit SoundPlayer(QObject* parent = nullptr);

    QString path_for(const Settings& s, const QString& kind = QStringLiteral("adhan")) const;
    void play(const QString& path, int fade_ms = 1500);
    void stop();

private slots:
    void step();

private:
    void ensure();

#if defined(VM_HAS_MULTIMEDIA)
    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_out = nullptr;
#endif
    bool m_tried = false;
    QTimer* m_fade = nullptr;
    double m_vol = 0.0;
    double m_inc = 0.05;
};

} // namespace vm
