#include "sound.h"
#include "paths.h"
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>
#include <algorithm>

namespace vm {

SoundPlayer::SoundPlayer(QObject* parent) : QObject(parent) {
    m_fade = new QTimer(this);
    m_fade->setInterval(60);
    connect(m_fade, &QTimer::timeout, this, &SoundPlayer::step);
}

void SoundPlayer::ensure() {
#if defined(VM_HAS_MULTIMEDIA)
    if (m_tried || !qEnvironmentVariable("VM_NO_AUDIO").isEmpty()) {
        return;
    }
    m_tried = true;
    try {
        m_player = new QMediaPlayer(this);
        m_out = new QAudioOutput(this);
        m_player->setAudioOutput(m_out);
    } catch (...) {
        qWarning() << "audio init failed";
        m_player = nullptr;
        m_out = nullptr;
    }
#endif
}

QString SoundPlayer::path_for(const Settings& s, const QString& kind) const {
    if (!s.sound_enabled) {
        return QString();
    }
    QString custom = s.adhan_sound;
    if (kind == QStringLiteral("pre")) {
        custom = s.pre_sound;
    } else if (kind == QStringLiteral("iqama")) {
        custom = s.iqama_sound;
    }
    if (!custom.isEmpty() && QFileInfo::exists(custom)) {
        return custom;
    }

    // Check original default wav names first
    QString wavName = QStringLiteral("chime.wav");
    if (kind == QStringLiteral("pre")) wavName = QStringLiteral("reminder.wav");
    else if (kind == QStringLiteral("iqama")) wavName = QStringLiteral("iqama.wav");
    QString wavPath = paths::asset(QStringLiteral("sounds"), wavName);
    if (QFileInfo::exists(wavPath)) {
        return wavPath;
    }

    // Fallback to bundled mp3 assets inside assets/sounds/ if wav files aren't present
    QString mp3Name = QStringLiteral("01 - Adhan Al-Haram.mp3");
    if (kind == QStringLiteral("pre")) {
        mp3Name = QStringLiteral("00 - Hayya 'alas-Salah.mp3");
    } else if (kind == QStringLiteral("iqama")) {
        mp3Name = QStringLiteral("02 - iqama.mp3");
    }
    QString mp3Path = paths::asset(QStringLiteral("sounds"), mp3Name);
    if (QFileInfo::exists(mp3Path)) {
        return mp3Path;
    }
    return wavPath;
}

void SoundPlayer::play(const QString& path, int fade_ms) {
    ensure();
#if defined(VM_HAS_MULTIMEDIA)
    if (!m_player || !m_out || path.isEmpty()) {
        return;
    }
    m_player->setSource(QUrl::fromLocalFile(path));
    m_out->setVolume(0.0f);
    m_vol = 0.0;
    m_inc = 1.0 / std::max(1, fade_ms / 60);
    m_player->play();
    m_fade->start();
#else
    Q_UNUSED(path);
    Q_UNUSED(fade_ms);
#endif
}

void SoundPlayer::step() {
#if defined(VM_HAS_MULTIMEDIA)
    m_vol = std::min(1.0, m_vol + m_inc);
    if (m_out) {
        m_out->setVolume(static_cast<float>(m_vol));
    }
    if (m_vol >= 1.0) {
        m_fade->stop();
    }
#endif
}

void SoundPlayer::stop() {
    m_fade->stop();
#if defined(VM_HAS_MULTIMEDIA)
    if (m_player) {
        m_player->stop();
    }
#endif
}

} // namespace vm
