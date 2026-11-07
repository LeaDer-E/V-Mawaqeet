"""Adhan / chime playback with a gentle fade-in. Fails silently if audio is unavailable."""
from __future__ import annotations
import logging, os
from pathlib import Path
from PySide6.QtCore import QObject, QTimer, QUrl
from .paths import asset
from .config import Settings

log = logging.getLogger("vm.sound")
try:
    from PySide6.QtMultimedia import QAudioOutput, QMediaPlayer
except Exception:  # pragma: no cover
    QAudioOutput = QMediaPlayer = None  # type: ignore


class SoundPlayer(QObject):
    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self.player = self.out = None
        self._tried = False
        self._fade = QTimer(self); self._fade.setInterval(60); self._fade.timeout.connect(self._step)
        self._vol = 0.0

    def _ensure(self) -> None:
        """Create the audio backend lazily (a missing audio device must never break startup)."""
        if self._tried or QMediaPlayer is None or os.environ.get("VM_NO_AUDIO"):
            return
        self._tried = True
        try:
            self.player = QMediaPlayer(self)
            self.out = QAudioOutput(self)
            self.player.setAudioOutput(self.out)
        except Exception:
            log.exception("audio init failed")
            self.player = None

    def path_for(self, s: Settings, kind: str = "adhan") -> Path | None:
        """kind: adhan | snooze | pre | iqama. Returns the user's file if set and present, else a built-in."""
        if not s.sound_enabled:
            return None
        custom = {"pre": s.pre_sound, "iqama": s.iqama_sound}.get(kind, s.adhan_sound)
        if custom and Path(custom).exists():
            return Path(custom)
        return asset("sounds", {"pre": "reminder.wav", "iqama": "iqama.wav"}.get(kind, "chime.wav"))

    def play(self, path: Path | None, fade_ms: int = 1500) -> None:
        self._ensure()
        if not self.player or not path:
            return
        try:
            self.player.setSource(QUrl.fromLocalFile(str(path)))
            self.out.setVolume(0.0); self._vol = 0.0
            self._inc = 1.0 / max(1, fade_ms // 60)
            self.player.play(); self._fade.start()
        except Exception:
            log.exception("play failed")

    def _step(self) -> None:
        self._vol = min(1.0, self._vol + self._inc)
        self.out.setVolume(self._vol)
        if self._vol >= 1.0:
            self._fade.stop()

    def stop(self) -> None:
        self._fade.stop()
        if self.player:
            self.player.stop()
