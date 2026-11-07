"""Run blocking callables on a thread pool and deliver results on the GUI thread."""
from __future__ import annotations
import logging
from typing import Any, Callable
from PySide6.QtCore import QObject, QRunnable, QThreadPool, Signal

log = logging.getLogger("vm.workers")


class _Sig(QObject):
    done = Signal(object, object)   # result, error


class _Job(QRunnable):
    def __init__(self, fn: Callable[[], Any], sig: _Sig) -> None:
        super().__init__()
        self.fn, self.sig = fn, sig

    def run(self) -> None:
        try:
            res, err = self.fn(), None
        except Exception as e:  # noqa: BLE001
            log.exception("worker failed")
            res, err = None, e
        try:
            self.sig.done.emit(res, err)
        except RuntimeError:        # receiver already destroyed (app shutting down)
            pass


_keep: set[_Sig] = set()


def run_async(fn: Callable[[], Any], on_done: Callable[[Any, Exception | None], None]) -> None:
    sig = _Sig()
    _keep.add(sig)

    def finish(res, err):
        _keep.discard(sig)
        on_done(res, err)
    sig.done.connect(finish)
    QThreadPool.globalInstance().start(_Job(fn, sig))
