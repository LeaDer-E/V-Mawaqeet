"""Build V-Mawaqeet into a Windows program.

    python build_exe.py                    -> dist/V-Mawaqeet/V-Mawaqeet.exe  (+ a "libs" folder next to it)
    python build_exe.py --contents NAME    -> use another name for that support folder (default: libs)
    python build_exe.py --onefile          -> one single V-Mawaqeet.exe, no support folder at all
                                              (slower to start, because it unpacks itself on every launch)
"""
import argparse, subprocess, sys
from pathlib import Path

root = Path(__file__).resolve().parent
ap = argparse.ArgumentParser()
ap.add_argument("--onefile", action="store_true", help="single .exe, no support folder")
ap.add_argument("--no-install", action="store_true", help="skip the pip install step")
ap.add_argument("--contents", default="libs", help='name of the support folder instead of PyInstaller\'s "_internal"')
args = ap.parse_args()

sep = ";" if sys.platform.startswith("win") else ":"
py = [sys.executable, "-m"]
if not args.no_install:
    subprocess.check_call(py + ["pip", "install", "-r", str(root / "requirements.txt"), "pyinstaller>=6.0"])
subprocess.check_call([sys.executable, str(root / "tools" / "make_icon.py")])          # logo -> assets/icon.ico

cmd = py + ["PyInstaller", "--noconfirm", "--clean", "--windowed",
            "--name", "V-Mawaqeet", "--icon", str(root / "assets" / "icon.ico"),
            "--add-data", f"assets{sep}assets", "--collect-data", "tzdata"]
for mod in ("QtWebEngineCore", "QtWebEngineWidgets", "QtWebEngineQuick", "QtPdf", "QtPdfWidgets", "Qt3DCore",
            "QtCharts", "QtDataVisualization", "QtDesigner", "QtHelp", "QtTest"):         # not used -> smaller build
    cmd += ["--exclude-module", f"PySide6.{mod}"]
cmd += ["--onefile"] if args.onefile else ["--contents-directory", args.contents]
cmd += ["main.py"]
subprocess.check_call(cmd, cwd=root)

out = root / "dist" / ("V-Mawaqeet.exe" if args.onefile else "V-Mawaqeet")
print("\nDone ->", out)
