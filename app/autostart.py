"""Launch-at-login (Windows registry Run key; no-op elsewhere)."""
from __future__ import annotations
import sys
from pathlib import Path

_KEY = r"Software\Microsoft\Windows\CurrentVersion\Run"
_NAME = "V-Mawaqeet"


def supported() -> bool:
    return sys.platform.startswith("win")


def _command() -> str:
    if getattr(sys, "frozen", False):
        return f'"{sys.executable}" --minimized'
    exe = Path(sys.executable)
    pyw = exe.with_name("pythonw.exe")
    main = Path(__file__).resolve().parent.parent / "main.py"
    return f'"{pyw if pyw.exists() else exe}" "{main}" --minimized'


def set_enabled(enabled: bool) -> bool:
    if not supported():
        return False
    import winreg
    try:
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, _KEY, 0, winreg.KEY_SET_VALUE) as k:
            if enabled:
                winreg.SetValueEx(k, _NAME, 0, winreg.REG_SZ, _command())
            else:
                try:
                    winreg.DeleteValue(k, _NAME)
                except FileNotFoundError:
                    pass
        return True
    except OSError:
        return False
