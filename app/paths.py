"""Filesystem locations (works from source and from a PyInstaller bundle)."""
from __future__ import annotations
import os, sys
from pathlib import Path


def resource_root() -> Path:
    if getattr(sys, "frozen", False):
        return Path(getattr(sys, "_MEIPASS", Path(sys.executable).parent))
    return Path(__file__).resolve().parent.parent


def asset(*parts: str) -> Path:
    return resource_root() / "assets" / Path(*parts)


def user_dir() -> Path:
    if sys.platform.startswith("win"):
        base = Path(os.environ.get("APPDATA", Path.home() / "AppData" / "Roaming"))
    elif sys.platform == "darwin":
        base = Path.home() / "Library" / "Application Support"
    else:
        base = Path(os.environ.get("XDG_CONFIG_HOME", Path.home() / ".config"))
    d = base / "V-Mawaqeet"
    d.mkdir(parents=True, exist_ok=True)
    return d


def cache_dir() -> Path:
    d = user_dir() / "cache"
    d.mkdir(parents=True, exist_ok=True)
    return d
