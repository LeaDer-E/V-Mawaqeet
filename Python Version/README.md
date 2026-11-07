<div align="center">

<img src="assets/icon.png" width="132" alt="V-Mawaqeet logo">

# V-Mawaqeet &nbsp;·&nbsp; مواقيت

**A premium, bilingual desktop prayer-times app - smart reminders, iqama alerts, 32+ themes.**
Built with Python and Qt (PySide6). Arabic (true RTL) and English. Windows & Linux.

![Version](https://img.shields.io/badge/version-2.6.0-d4a73e)
![Python](https://img.shields.io/badge/python-3.10%2B-3776ab)
![Qt](https://img.shields.io/badge/PySide6-Qt%206-41cd52)
![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-555)
![License](https://img.shields.io/badge/license-MIT-blue)

<img src="docs/screenshots/home-dark-rtl.png" width="900" alt="V-Mawaqeet home screen (dark, Arabic RTL)">

</div>

---

## Table of contents

1. [Highlights](#highlights)
2. [Screenshots](#screenshots)
3. [Features in detail](#features-in-detail)
4. [Quick start](#quick-start) - [Windows](#windows) · [Linux](#linux) · [macOS](#macos)
5. [Build a standalone application](#build-a-standalone-application) - [Windows `.exe` + installer](#windows-exe--installer) · [Linux binary + app-menu entry](#linux-binary--app-menu-entry)
6. [Using the app](#using-the-app)
7. [Configuration reference](#configuration-reference)
8. [Prayer-time engine](#prayer-time-engine)
9. [Architecture](#architecture)
10. [Design system](#design-system)
11. [Testing](#testing)
12. [Troubleshooting](#troubleshooting)
13. [Privacy & network use](#privacy--network-use)
14. [Known limitations](#known-limitations)
15. [Releasing](#releasing) · [Contributing](#contributing) · [Credits & licenses](#credits--licenses) · [Author](#author)

---

## Highlights

- **Never blocks, never empty.** Times appear instantly from a cache or a built-in astronomical engine; the online source refreshes in the background. Offline works out of the box.
- **Smart notifications.** Per-prayer **adhan alarm**, **reminder before** the adhan (own minutes per prayer), **iqama alert** after it (own minutes per prayer), **snooze** with any number of minutes, and a separate sound file for each type.
- **Premium look.** Frameless window, hand-painted widgets (no stock OS controls), glass surfaces, soft shadows, animated transitions, procedurally painted illustration for every prayer.
- **32 themes + your own colour.** Dark / Light / Auto, each palette generated for both modes. The logo, tray icon, countdown and pop-ups follow the theme.
- **True bilingual UI.** Arabic switches the *whole layout* to right-to-left, not just the text. Live language switching, no restart.
- **Global coverage.** 250 countries, 4,144 regions with Arabic & English names; 23 calculation methods with an automatic per-country default.
- **Lives in the tray.** Live countdown, today's times, quick navigation and switches right from the tray menu.
- **~5,000 lines of readable, typed Python.** Small modules, no hidden magic, headless test-suite included.

## Screenshots

| | |
|---|---|
| ![Light theme, English, royal-blue palette](docs/screenshots/home-light-ltr.png) | ![Themes gallery](docs/screenshots/themes.png) |
| *Light mode · English · Royal Blue palette* | *Themes page - 32 palettes + custom colour* |
| ![Settings](docs/screenshots/settings.png) | ![About dialog](docs/screenshots/about.png) |
| *Settings - per-prayer alarm / reminder / iqama* | *About dialog* |

![Notifications: adhan (with snooze) and iqama](docs/screenshots/notifications.png)
*Notifications use the prayer's own scene as background. Left: adhan with quick and custom snooze. Right: compact iqama alert.*

![Tray menu in English and Arabic](docs/screenshots/tray-menu.png)
*Tray menu (English / Arabic): live countdown, today's times, navigation, alerts, theme & language switches.*

> I Hope i have some time to add the screenshots.

## Features in detail

### Prayer times
- Fajr, Sunrise, Dhuhr, Asr, Maghrib, Isha with Gregorian **and Hijri** date.
- Source chain: **cache → AlAdhan API (coordinates) → AlAdhan API (city) → local astronomical calculation.** Each online call has a timeout and retry with exponential back-off, always on a worker thread.
- **23 calculation methods** (Egyptian, Umm Al-Qura, MWL, ISNA, Karachi, Kuwait, Qatar, Gulf, Dubai, Jordan, Diyanet, Tehran, Jafari, Singapore, Malaysia, Indonesia, Morocco, Algeria, Tunisia, France, Russia, Portugal, Moonsighting) with a **default suggested per country**.
- Asr school (standard / Hanafi) and **manual ±minutes per prayer**.
- Month-long cache → the app keeps working without internet.

### Location
- **First run asks you**: detect automatically (OS location → IP fallback) or choose manually.
- Auto-detection maps coordinates to the **nearest region across all countries** (not the nearest country centre), and shows what it found so you can correct it.
- Searchable country and region pickers, Arabic and English names.

### Notifications
| Type | When | Configurable per prayer | Sound |
|---|---|---|---|
| **Adhan alarm** | at the prayer time | on / off | your file, or built-in chime |
| **Reminder** | *N* minutes **before** the adhan | on / off + minutes | your file, or built-in reminder tone |
| **Iqama** | *N* minutes **after** the adhan (not for sunrise) | on / off + minutes | your file, or built-in iqama tone |
| **Snooze** | after you snooze | - | uses the adhan sound |

- Pop-up (bottom-right, always on top) with the prayer's scene as backdrop; **Stop / Open app / Snooze 5-10-15 / Snooze any minutes**.
- Master switch, default snooze length, sound on/off, 90-second tolerance window (a late tick or a short sleep never swallows an alert).
- A **bell on every prayer card** mutes / unmutes that prayer's alarm in one click.

### Interface
- **Home**: header with city and Gregorian + Hijri date, a hero banner for the **next prayer** (its scene as backdrop + frosted HH:MM:SS tiles + progress bar), and six mihrab-shaped prayer cards (past = dimmed with ✓, next = glowing with a badge).
- **Friday**: the Dhuhr slot reads **Jumu'ah / الجمعة** automatically (by the selected place's local date).
- **Themes page**, **Settings page**, **About dialog**, toast messages, animated page and theme transitions.
- Hover-expanding icon sidebar, custom title bar, native-feeling resize and drag on a frameless window.

### System integration
- **System tray**: closing the window keeps the app running (optional). Tray menu: next-prayer countdown, today's times, Home / Settings / Themes / About, alerts on/off, cancel snooze, switch theme, switch language, quit. Tooltip shows time remaining.
- **Launch at Windows startup** (registry, per user). `--minimized` starts hidden.
- **Single instance** guard.
- Errors are logged to `app.log`; settings are stored per user.

## Quick start

**Requirements:** Python **3.10+** (developed and tested on 3.12), `pip`, internet access only for the first live refresh (the app works offline).

```text
PySide6>=6.7      Qt 6 bindings (UI, multimedia, positioning)
requests>=2.31    HTTP client for the prayer-time and geolocation APIs
tzdata>=2024.1    IANA time-zone database (needed on Windows)
```

### Windows
```powershell
git clone https://github.com/<you>/V-Mawaqeet.git
cd V-Mawaqeet
python -m venv .venv
.\.venv\Scripts\Activate.ps1          # cmd.exe:  .venv\Scripts\activate.bat
python -m pip install -r requirements.txt
python main.py
```
> Always use **`python -m pip`** (not bare `pip`) so packages go to the same Python that runs the app. Mixing the two is the most common reason for `ModuleNotFoundError: No module named 'PySide6'`.

### Linux
1. Install Python and the Qt runtime libraries (typical package names):

   | Distro | Command |
   |---|---|
   | Ubuntu / Debian | `sudo apt install python3 python3-venv python3-pip libxcb-cursor0 libxkbcommon-x11-0 libegl1 libgl1 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libxcb-xinerama0` |
   | Fedora | `sudo dnf install python3 python3-pip xcb-util-cursor libxkbcommon-x11 mesa-libEGL` |
   | Arch | `sudo pacman -S python python-pip xcb-util-cursor libxkbcommon-x11` |

2. Run:
   ```bash
   git clone https://github.com/<you>/V-Mawaqeet.git
   cd V-Mawaqeet
   python3 -m venv .venv && source .venv/bin/activate
   python -m pip install -r requirements.txt
   python main.py
   ```
- **Tray on GNOME** needs the *AppIndicator and KStatusNotifierItem Support* extension. KDE, XFCE, Cinnamon, MATE work out of the box.
- **Launch at login** is Windows-only for now. On Linux copy the installed `.desktop` file to `~/.config/autostart/` and add `--minimized` to its `Exec=` line.
- Designed for X11 and Wayland (the frameless window uses Qt's native system move/resize). reports are welcome.

### macOS
The code is cross-platform Qt and the steps are the same as Linux, but **macOS has not been tested** and there is no packaging for it yet.

## Build a standalone application

PyInstaller does not cross-compile: **build on the OS you target.**

### Windows `.exe` + installer
```powershell
python -m pip install -r requirements-dev.txt
python build_exe.py
```
Result:
```text
dist\V-Mawaqeet\
├── V-Mawaqeet.exe        <- double-click (icon = the V-minaret logo)
└── libs\                 <- support files (PyInstaller calls this folder "_internal")
    └── assets\ ...
```

| Command | What you get |
|---|---|
| `python build_exe.py` | folder build: `V-Mawaqeet.exe` + `libs\` |
| `python build_exe.py --contents data` | same, but the support folder is called `data` (any name works) |
| `python build_exe.py --onefile` | a **single** `V-Mawaqeet.exe`, no support folder (starts slower: it unpacks itself each launch; antivirus tools flag one-file builds more often) |
| `python build_exe.py --no-install` | skip the `pip install` step (already prepared environment) |

What the script does: installs dependencies → **renders the logo into `assets/icon.ico`** (`tools/make_icon.py`) → runs PyInstaller (`--windowed`, bundles `assets/` and `tzdata`, excludes unused Qt modules) → prints the output path. A Linux test build measured about **224 MB** on disk - the Qt runtime dominates.

**Make a real installer (optional).** Install [Inno Setup 6](https://jrsoftware.org/isinfo.php), build once with `python build_exe.py`, then compile `packaging/windows/installer.iss` (open it and press *Compile*, or `iscc packaging\windows\installer.iss`). You get `dist\installer\V-Mawaqeet-Setup-<version>.exe` with Start-menu entry, optional desktop shortcut and uninstaller. *The script is provided as a template and has not been run on every Windows setup.*

**Tips**
- Prefer the **folder build** (or the installer) over `--onefile` for fewer antivirus false positives and faster start-up.
- Unsigned executables trigger **SmartScreen** the first time. Code-signing certificates remove the warning. But of course I will not buy one :D
- If you change the `app ico` If Explorer keeps showing an old icon after rebuilding, it is the Windows icon cache: restart Explorer or copy the exe elsewhere.

### Linux binary + app-menu entry
```bash
python -m pip install -r requirements-dev.txt
python build_exe.py                    # -> dist/V-Mawaqeet/V-Mawaqeet  (+ libs/)
./packaging/linux/install.sh           # installs for the current user, no root
```
The installer copies the build to `~/.local/opt/V-Mawaqeet`, adds the `v-mawaqeet` command to `~/.local/bin`, installs the icon and an application-menu entry. Remove it with `./packaging/linux/install.sh --uninstall` (your settings are kept).

To distribute: `tar -czf V-Mawaqeet-linux-x86_64.tar.gz -C dist V-Mawaqeet` and ship `packaging/linux/` alongside it. Build on the **oldest** distribution you want to support (glibc compatibility). An AppImage can be produced from the folder build with `appimagetool` - not included.

## Using the app

**First run.** A short wizard asks for language → how to find your location (automatic or manual) → calculation method (a sensible default for your country is pre-selected) → theme. Nothing else is needed.

**Home.**
- The hero banner shows the next prayer, the frosted **hours / minutes / seconds** tiles and a progress bar from the previous prayer to the next.
- Tap the **bell** on a card to mute or unmute that prayer's alarm.
- Header: language switch (عربي | EN) and a light/dark button.

**Settings.**
| Section | What you can do |
|---|---|
| Location | automatic detection (shows the detected place), or pick country and region |
| Notifications | master switch, default snooze minutes, sound on/off, files for adhan / reminder / iqama (with preview and reset) |
| Per-prayer alerts | for every prayer: alarm on/off · reminder on/off + minutes before · iqama on/off + minutes after |
| Sources | auto (online → local) or local only; live status badges; calculation method; Asr school; ±minutes per prayer; *Refresh times* |
| Appearance | language, open the Themes page |
| System | launch at Windows startup, keep running in tray when closed |

**Themes.** Choose *Dark*, *Light* or *Auto* (follows the OS), then one of 32 palettes - or **Your own colour** (any colour; dark and light variants are generated for you).

**Tray menu.** Right-click the tray icon: live countdown to the next prayer, *Today's times*, Open / Home / Settings / Themes / About, alerts on-off, cancel snooze, switch theme, switch language, Quit.

**Command line.** `python main.py --minimized` (or `V-Mawaqeet.exe --minimized`) starts hidden in the tray.

## Configuration reference

Settings are plain JSON, written on every change.

| OS | Folder |
|---|---|
| Windows | `%APPDATA%\V-Mawaqeet\` |
| Linux | `~/.config/V-Mawaqeet/` (respects `XDG_CONFIG_HOME`) |
| macOS | `~/Library/Application Support/V-Mawaqeet/` |

Inside: `config.json` · `cache\` (monthly prayer-time files) · `app.log` · `app.lock`. **Reset the app** by closing it and deleting the folder.

| Key | Default | Meaning |
|---|---|---|
| `first_run` | `true` | show the onboarding wizard |
| `language` | `"ar"` | `ar` or `en` |
| `theme` / `palette` / `custom_accent` | `"dark"` / `"gold"` / `"#E6B85C"` | `dark`·`light`·`auto`; a palette key or `custom`; colour used by `custom` |
| `auto_location` | `false` | detect location instead of the chosen region |
| `country_code` / `region_index` | `"EG"` / `0` | selected country (ISO-2) and region index |
| `lat` / `lng` / `tz` | Alexandria | coordinates and IANA time zone actually used |
| `method` | `"auto"` | calculation method key or `auto` (per-country default) |
| `asr_school` | `0` | `0` standard, `1` Hanafi |
| `offsets` | all `0` | manual minutes per prayer |
| `source_mode` | `"auto"` | `auto` (online → local) or `offline` |
| `notify` | `true` | master switch for all alerts |
| `alerts` | all on except sunrise | adhan alarm per prayer |
| `reminders` / `reminder_min` | all off / `10` | reminder per prayer / minutes **before** |
| `iqama` / `iqama_min` | all off / fajr 20, dhuhr 15, asr 15, maghrib 10, isha 15 | iqama per prayer / minutes **after** |
| `snooze_minutes` | `5` | default value in the snooze box |
| `sound_enabled` | `true` | play sounds |
| `adhan_sound` / `pre_sound` / `iqama_sound` | `""` | file path, empty = built-in sound |
| `autostart` | `false` | launch at Windows startup |
| `minimize_to_tray` | `true` | closing the window hides it |

Older settings files are **migrated automatically** (e.g. the former single global reminder becomes per-prayer reminders). `notify_sunrise` and `pre_reminder` are legacy keys kept only for that migration.

## Prayer-time engine

```mermaid
flowchart LR
    A[Request for a month] --> B{Cache?}
    B -- hit --> Z[Timetable]
    B -- miss --> L[Local calculation: instant]
    L --> Z
    B -- miss --> C[AlAdhan by coordinates]
    C -- ok --> D[(Save month to cache)] --> Z
    C -- fail --> E[AlAdhan by city name]
    E -- ok --> D
    E -- fail --> L
```
- The UI is filled **immediately** from cache or the local engine; the online fetch runs in the background and replaces the data when it arrives (with a toast).
- The local engine is an original Python implementation of the standard solar-position algorithm (as described by PrayTimes.org): Fajr/Isha by twilight angle (or fixed minutes after Maghrib for Umm Al-Qura-style methods), Asr by shadow factor, a **one-seventh-of-the-night rule** when twilight never ends at high latitudes. In a spot check for Alexandria it matched AlAdhan's Sunrise, Asr and Maghrib to the minute.
- Hijri dates come from AlAdhan when online and from a tabular algorithm when offline (can differ by a day from moon sighting).
- Always compare with your local religious authority; use *manual adjustment* if your mosque differs by a minute or two.

<details>
<summary><b>Calculation methods</b> (key · AlAdhan id · Fajr° · Isha)</summary>

| Key | Id | Fajr | Isha | Name |
|---|---|---|---|---|
| `egypt` | 5 | 19.5° | 17.5° | Egyptian General Authority of Survey |
| `ummalqura` | 4 | 18.5° | 90 min | Umm Al-Qura University, Makkah |
| `mwl` | 3 | 18° | 17° | Muslim World League |
| `isna` | 2 | 15° | 15° | Islamic Society of North America |
| `karachi` | 1 | 18° | 18° | University of Islamic Sciences, Karachi |
| `kuwait` | 9 | 18° | 17.5° | Kuwait |
| `qatar` | 10 | 18° | 90 min | Qatar |
| `gulf` | 8 | 19.5° | 90 min | Gulf Region |
| `dubai` | 16 | 18.2° | 18.2° | Dubai |
| `jordan` | 23 | 18° | 18° | Jordan Ministry of Awqaf |
| `turkey` | 13 | 18° | 17° | Diyanet İşleri Başkanlığı |
| `tehran` | 7 | 17.7° | 14° | Institute of Geophysics, Univ. of Tehran |
| `jafari` | 0 | 16° | 14° | Shia Ithna-Ashari (Leva, Qum) |
| `singapore` | 11 | 20° | 18° | MUIS |
| `malaysia` | 17 | 20° | 18° | JAKIM |
| `indonesia` | 20 | 20° | 18° | Kemenag |
| `morocco` | 21 | 19° | 17° | Morocco |
| `algeria` | 19 | 18° | 17° | Algeria |
| `tunisia` | 18 | 18° | 18° | Tunisia |
| `france` | 12 | 12° | 12° | UOIF |
| `russia` | 14 | 16° | 15° | Spiritual Administration of Muslims of Russia |
| `portugal` | 22 | 18° | 77 min | Portugal |
| `moonsighting` | 15 | 18° | 18° | Moonsighting Committee Worldwide |

</details>

## Architecture

```mermaid
flowchart TB
    subgraph UI [app/ui - painted widgets, no business logic]
        W[window.py<br/>frameless shell · tray · sidebar] --> H[home.py] & S[settings_page.py] & T[themes_page.py] & O[onboarding.py] & A[about_dialog.py]
        N[notification.py] --- WID[widgets.py · icons.py · art.py]
    end
    subgraph Core [app/ - Qt objects without widgets]
        ST[state.py<br/>AppState] --> P[providers.py<br/>cache · AlAdhan · fallback]
        P --> PC[prayer_calc.py<br/>local engine]
        ST --> NO[notifier.py<br/>1 s scheduler]
        CFG[config.py<br/>Settings JSON] --- ST
        TH[theme.py<br/>tokens · palettes] --- I18[i18n.py]
        WK[workers.py<br/>thread pool]
        SND[sound.py] --- GEO[geo.py]
    end
    W <--> ST
    NO -->|fire: key, kind| W
    W --> N
    ST --> WK
```

| Path | Responsibility |
|---|---|
| `main.py` | entry point: logging, single-instance lock, fonts, settings, window |
| `app/config.py` | `Settings` dataclass ⇄ JSON, with migrations |
| `app/state.py` | `AppState`: location, timetable, snapshot of "next prayer", refresh, auto-location mapping |
| `app/providers.py` | monthly cache + AlAdhan clients + fallback chain (blocking; runs on workers) |
| `app/prayer_calc.py` · `hijri.py` | local solar engine · tabular Hijri converter |
| `app/methods.py` | 23 methods, per-country defaults |
| `app/notifier.py` | decides *when* adhan / reminder / iqama / snooze fire (pure logic, testable) |
| `app/workers.py` | `run_async(fn, on_done)` - result delivered on the GUI thread |
| `app/sound.py` | `QMediaPlayer` with fade-in; audio backend created lazily |
| `app/geo.py` · `ui/locate.py` | OS location (Qt Positioning) → IP geolocation fallback |
| `app/locations.py` + `assets/data/locations.json` | 250 countries / 4,144 regions, AR + EN |
| `app/theme.py` | design tokens, 32 generated palettes, QSS |
| `app/i18n.py` | AR / EN string table, RTL helpers, Jumu'ah logic |
| `app/ui/widgets.py` | Button, Toggle, Segmented, SearchCombo, MinutesInput, Toast, SlideStack … |
| `app/ui/icons.py` | all icons **and the logo** drawn with `QPainter` |
| `app/ui/art.py` | the six prayer scenes (sky, sun/moon, mosque, palms, reflection) |
| `app/ui/window.py` | frameless window, title bar, sidebar, tray menu, theme / language rebuild |
| `tools/` | `make_icon.py` (logo → `.ico`), `build_locations.py` (rebuild the location DB) |
| `tests/` | headless test-suite |
| `packaging/` | Linux installer + `.desktop`, Inno Setup template |

**Key design decisions**
- **Everything visible is painted** with `QPainter` instead of styled stock widgets → identical look on every OS, pixel-level control, easy animation.
- **Theme = data.** Every colour is a token; painted widgets read tokens at paint time. A theme/language switch rebuilds the pages behind a cross-fade.
- **Network and disk-heavy work never touches the UI thread** (`QThreadPool` + signals).
- **Offline-first.** Show something instantly, improve it asynchronously.
- **Notifier is pure logic** (a 1 s timer comparing "now" to scheduled times) so it can be tested by faking the clock.

## Design system

- **Palettes**: 8 hand-tuned + 24 generated from one accent via HSL, each with dark and light variants; `custom` generates a palette from any colour. Tokens: `bg0 bg1 bg2 surface border text text2 accent accent2 on_accent …`.
- **Typography**: [Tajawal](https://fonts.google.com/specimen/Tajawal) (static weights; Arabic + Latin), bundled.
- **Shape & motion**: radii 12–30 px, 8 px spacing grid, 160–320 ms ease-out transitions, layered soft shadows, glass surfaces in dark mode.
- **Imagery**: prayer illustrations are generated by code (layered sky → sun/moon → clouds → skyline → mosque → ground → palms → reflection), so they scale to any size and never ship as image files.
- **Logo**: drawn from the active palette (dark badge + metallic accent); same function feeds the sidebar, title bar, tray, About dialog and the `.ico`.
- **RTL**: Arabic sets Qt's layout direction to right-to-left; custom painting handles mirrored alignment explicitly.

## Testing

Tests are **headless** (`QT_QPA_PLATFORM=offscreen`) and fake the clock where time matters.

```bash
python tests/run_all.py            # everything
python tests/test_alerts.py        # a single file
```
| File | Covers |
|---|---|
| `test_alerts.py` | per-prayer reminders and iqama minutes, "fires once", sounds, config migrations, popup layout |
| `test_friday.py` | Dhuhr ↔ Jumu'ah across views, languages and midnight |
| `test_about.py` | open/close, ESC, clipboard, all language × theme combinations |
| `test_tray.py` | tray menu logic (with a stand-in tray), live countdown, navigation |

**Manual checklist** for what headless tests cannot see: real tray icon & menu, launch at startup, OS location permission prompt, actual audio output, high-DPI rendering, installer.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `ModuleNotFoundError: No module named 'PySide6'` | `pip` and `python` point to different installs. Use `python -m pip install -r requirements.txt`, or activate your venv first. |
| Linux: *"could not load the Qt platform plugin xcb"* | install `libxcb-cursor0` (and the packages in the Linux table). `QT_DEBUG_PLUGINS=1 python main.py` shows what is missing. |
| No tray icon on GNOME | install/enable the *AppIndicator and KStatusNotifierItem Support* extension. |
| No sound | check **Settings → Notifications → Play sound**; use the ▶ preview. Supported by Qt's FFmpeg backend: mp3, wav, ogg, flac, m4a, aac. On a machine without an audio device the app stays silent instead of crashing (`VM_NO_AUDIO=1` forces silence). |
| Auto-detect picks a neighbouring city | IP geolocation is approximate and coordinates are region-level; near borders the nearest region can be across the line. Switch off auto-detect and choose manually. |
| Times differ by 1–2 minutes from my mosque | pick the method your authority uses, or add a manual ± adjustment per prayer. |
| Windows SmartScreen / antivirus warning | the executable is unsigned. Use the folder build or the installer, and sign the binaries if you distribute widely. |
| Old icon after rebuilding | Windows icon cache - restart Explorer or move the file. |
| App does not start the second time | single-instance guard. Close the running instance (tray → Quit) or delete `app.lock` in the data folder. |
| Want a clean slate | close the app and delete the data folder (see *Configuration reference*). |

## Privacy & network use

- **No telemetry, no accounts, no ads.** Everything is stored locally.
- Online requests are limited to: **api.aladhan.com** (your coordinates or city name, to fetch prayer times) and - only if you use automatic location - **ipwho.is / ip-api.com / ipapi.co** (your public IP is visible to them, as with any web request). Turn on *Local calculation only* in Settings → Sources for zero network use for prayer times.
- OS location (GPS / Wi-Fi) is requested only when you choose automatic detection.

## Known limitations

- The adhan **audio is not bundled** (licensing); choose your own file. Built-in sounds are short synthesized tones.
- Region coordinates are **governorate/state level**, not street level.
- Launch-at-login is **Windows-only**; the tray menu uses the OS native menu style (not themeable); **macOS untested**.
- Offline Hijri dates use a tabular algorithm.
- The **iqama sound** is one file for all prayers (minutes are per prayer).

## Releasing

1. Bump `APP_VERSION` in `app/__init__.py` and `AppVersion` in `packaging/windows/installer.iss`; update `CHANGELOG.md`.
2. `python tests/run_all.py`
3. On **Windows**: `python build_exe.py` → (optional) compile the Inno Setup script → zip `dist\V-Mawaqeet` or ship the installer.
4. On **Linux**: `python build_exe.py` → `tar -czf …`.
5. Tag `vX.Y.Z`, create a GitHub Release, upload the artifacts and the changelog entry.

## Contributing

Issues and pull requests are welcome. Please keep the style: type hints, small modules, **no stock-looking widgets**, every colour from a theme token, every user-visible string in `app/i18n.py` in **both** languages, and add or extend a headless test for logic changes.

## Credits & licenses

| Component | License |
|---|---|
| This project | **MIT** (`LICENSE`) |
| [Qt for Python (PySide6)](https://www.qt.io/qt-for-python) | LGPL-3.0 / GPL / commercial - if you redistribute a bundled build, review Qt's LGPL obligations ([qt.io/qt-licensing](https://www.qt.io/qt-licensing)). The folder build keeps Qt as separate shared libraries. *Not legal advice.* |
| [Tajawal](https://fonts.google.com/specimen/Tajawal) font | SIL Open Font License 1.1 (`assets/fonts/OFL.txt`) |
| [countries-states-cities-database](https://github.com/dr5hn/countries-states-cities-database) (source of `locations.json`) | ODbL 1.0 - attribution required |
| [AlAdhan API](https://aladhan.com/prayer-times-api) | free API, subject to its terms of use |
| `requests`, `tzdata` | Apache-2.0 |
| Local calculation | original implementation of the published solar algorithm (see PrayTimes.org) |
| Icons, logo, prayer scenes | drawn in code for this project |

## Author

**Eslam Mustafa** - <eslaam.mustafa@gmail.com>

Developed with the assistance of Claude. *Made with care, for a meaningful daily reminder.*

> نسأل الله أن يجعل هذا العمل خالصًا لوجهه، وأن ينفع به، وأن يكتب لنا ولكم الأجر.
