"""Smart notification scheduler: adhan, pre-reminders and snooze."""
from __future__ import annotations
from datetime import datetime, timedelta
from PySide6.QtCore import QObject, QTimer, Signal

from .state import AppState


class Notifier(QObject):
    fire = Signal(str, str, object)      # prayer key, kind (adhan|pre|snooze), datetime
    snoozeChanged = Signal()

    def __init__(self, state: AppState) -> None:
        super().__init__()
        self.state = state
        self._fired: set[tuple] = set()
        self.snoozes: dict[str, datetime] = {}
        self._t = QTimer(self); self._t.setInterval(1000); self._t.timeout.connect(self.tick)
        self._t.start()

    def snooze(self, key: str, minutes: int) -> None:
        self.snoozes[key] = self.state.now() + timedelta(minutes=minutes)
        self.snoozeChanged.emit()

    def cancel_snooze(self) -> None:
        self.snoozes.clear(); self.snoozeChanged.emit()

    def next_snooze(self) -> tuple[str, datetime] | None:
        if not self.snoozes:
            return None
        k = min(self.snoozes, key=self.snoozes.get)
        return k, self.snoozes[k]

    @staticmethod
    def _due(now: datetime, when: datetime) -> bool:
        """True during the 90 seconds that start at `when` (tolerates a late tick or a short sleep)."""
        return 0 <= (now - when).total_seconds() < 90

    def tick(self) -> None:
        s = self.state.s
        if not s.notify:
            return
        now = self.state.now()
        for key, until in list(self.snoozes.items()):
            if now >= until:
                del self.snoozes[key]
                self.snoozeChanged.emit()
                self.fire.emit(key, "snooze", until)
        for key, dt in self.state.day_prayers(now.date()):
            d = dt.date()
            # 1) the adhan alarm
            if s.alerts.get(key, key != "sunrise") and self._due(now, dt) and (d, key, "adhan") not in self._fired:
                self._fired.add((d, key, "adhan")); self.fire.emit(key, "adhan", dt)
            # 2) reminder N minutes before the adhan (N is set per prayer)
            pm = int(s.reminder_min.get(key, 10) or 0)
            if s.reminders.get(key, False) and pm > 0 and self._due(now, dt - timedelta(minutes=pm)) and (d, key, "pre") not in self._fired:
                self._fired.add((d, key, "pre")); self.fire.emit(key, "pre", dt)
            # 3) iqama N minutes after the adhan (N is set per prayer; not for sunrise)
            im = int(s.iqama_min.get(key, 15) or 0)
            if key != "sunrise" and s.iqama.get(key, False) and im > 0 and self._due(now, dt + timedelta(minutes=im)) and (d, key, "iqama") not in self._fired:
                self._fired.add((d, key, "iqama")); self.fire.emit(key, "iqama", dt)
        if len(self._fired) > 200:
            self._fired = {f for f in self._fired if f[0] >= now.date()}
