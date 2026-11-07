"""Location detection for the UI: OS location services first, IP geolocation as fallback."""
from __future__ import annotations
import logging
from typing import Callable
from PySide6.QtCore import QObject, QTimer

from .. import geo
from ..state import AppState

log = logging.getLogger("vm.locate")


class Locator(QObject):
    def __init__(self, state: AppState, parent=None) -> None:
        super().__init__(parent)
        self.state = state
        self._cb: Callable | None = None
        self._src = None
        self._done = True
        self._timeout = QTimer(self); self._timeout.setSingleShot(True); self._timeout.timeout.connect(self._fallback)

    def detect(self, cb: Callable) -> None:
        """cb((country, region_index, fix) | None) is called on the GUI thread."""
        self._cb, self._done = cb, False
        try:
            from PySide6.QtPositioning import QGeoPositionInfoSource
            src = QGeoPositionInfoSource.createDefaultSource(self)
            if src is None:
                return self._fallback()
            self._src = src
            src.positionUpdated.connect(self._got_pos)
            src.errorOccurred.connect(lambda *_: self._fallback())
            src.requestUpdate(6000)
            self._timeout.start(7000)
        except Exception:
            log.info("OS location unavailable, using IP")
            self._fallback()

    def _got_pos(self, info) -> None:
        if self._done:
            return
        c = info.coordinate()
        if not c.isValid():
            return self._fallback()
        self._finish(self.state.apply_fix(geo.GeoFix(c.latitude(), c.longitude(), geo.reverse_country(c.latitude(), c.longitude()), "gps")))

    def _fallback(self) -> None:
        if self._done:
            return
        self._timeout.stop()
        self._done = True
        self.state.detect_location(lambda res: self._cb and self._cb(res))

    def _finish(self, res) -> None:
        self._done = True
        self._timeout.stop()
        if self._cb:
            self._cb(res)
