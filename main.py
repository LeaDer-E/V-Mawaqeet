"""V-Mawaqeet — entry point."""
from __future__ import annotations
import logging, sys

from PySide6.QtCore import QLockFile, Qt
from PySide6.QtGui import QFont, QFontDatabase
from PySide6.QtWidgets import QApplication

from app import APP_NAME, i18n
from app.config import Settings
from app.paths import asset, user_dir


def main() -> int:
    logging.basicConfig(filename=str(user_dir() / "app.log"), level=logging.INFO,
                        format="%(asctime)s %(levelname)s %(name)s: %(message)s")
    if sys.platform.startswith("win"):
        try:
            import ctypes
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID("VMawaqeet.App")
        except Exception:
            pass
    app = QApplication(sys.argv)
    app.setApplicationName(APP_NAME)
    app.setQuitOnLastWindowClosed(False)
    lock = QLockFile(str(user_dir() / "app.lock"))
    if not lock.tryLock(100):
        return 0                                       # already running
    for f in sorted(asset("fonts").glob("*.ttf")):
        QFontDatabase.addApplicationFont(str(f))
    app.setFont(QFont("Tajawal", 10))
    settings = Settings.load()
    i18n.set_language(settings.language)
    from app.ui.window import MainWindow               # imported after the QApplication exists
    win = MainWindow(settings, start_hidden="--minimized" in sys.argv and not settings.first_run)
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
