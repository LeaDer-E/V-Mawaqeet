"""Two-language string table (Arabic / English)."""
from __future__ import annotations

_lang = "ar"
S: dict[str, tuple[str, str]] = {  # key: (ar, en)
    "app": ("مواقيت", "V-Mawaqeet"),
    "home": ("الرئيسية", "Home"), "settings": ("الإعدادات", "Settings"),
    "next_prayer": ("الصلاة القادمة", "Next prayer"),
    "remaining": ("الوقت المتبقي", "Time remaining"),
    "upcoming": ("القادمة", "NEXT"),
    "fajr": ("الفجر", "Fajr"), "sunrise": ("الشروق", "Sunrise"), "dhuhr": ("الظهر", "Dhuhr"),
    "jumuah": ("الجمعة", "Jumu'ah"), "asr": ("العصر", "Asr"), "maghrib": ("المغرب", "Maghrib"), "isha": ("العشاء", "Isha"),
    "source": ("المصدر", "Source"), "method": ("طريقة الحساب", "Calculation method"),
    "snoozed": ("غفوة", "Snoozed"),
    # settings
    "sec_location": ("الموقع", "Location"), "sec_alerts": ("التنبيهات", "Notifications"),
    "sec_sources": ("مصادر المواقيت", "Prayer-time sources"), "sec_look": ("المظهر", "Appearance"),
    "sec_calc": ("الحساب والتعديل اليدوي", "Calculation & fine-tuning"),
    "sec_system": ("النظام", "System"),
    "auto_loc": ("تحديد الموقع تلقائياً (GPS / IP)", "Detect my location (GPS / IP)"),
    "detect_now": ("حدّد الآن", "Detect now"), "detecting": ("جارٍ تحديد موقعك…", "Finding your location…"),
    "country": ("الدولة", "Country"), "region": ("المحافظة / الولاية", "Region / State"),
    "search": ("ابحث…", "Search…"), "no_results": ("لا توجد نتائج", "No results"),
    "enable_alerts": ("تفعيل تنبيهات الأذان", "Enable adhan alerts"),
    "alert_sunrise": ("تنبيه وقت الشروق", "Alert at sunrise"),
    "snooze_len": ("مدة الغفوة", "Snooze length"), "minutes": ("دقيقة", "min"),
    "pre_rem": ("تذكير قبل الصلاة", "Reminder before prayer"), "off": ("إيقاف", "Off"),
    "sound": ("صوت التنبيه", "Alert sound"), "chime": ("جرس هادئ", "Soft chime"),
    "custom": ("ملف خاص", "Custom file"), "silent": ("صامت", "Silent"),
    "choose_file": ("اختيار ملف الأذان…", "Choose adhan file…"),
    "preview": ("استماع", "Preview"), "test_notif": ("تجربة الإشعار", "Test notification"),
    "primary_src": ("وضع المصدر", "Source mode"),
    "src_auto": ("تلقائي (إنترنت ← حساب محلي)", "Auto (online → local)"),
    "src_offline": ("حساب محلي فقط", "Local calculation only"),
    "src_local": ("حساب محلي (Offline)", "Local calculation (offline)"),
    "priority": ("ترتيب الأولوية", "Fallback order"),
    "connected": ("متصل", "Connected"), "failed": ("فشل", "Failed"),
    "offline_ready": ("جاهز دائماً", "Always ready"), "cached": ("من الذاكرة", "Cached"),
    "idle": ("لم يُستخدم", "Unused"),
    "school": ("المذهب (العصر)", "Asr school"), "school0": ("الجمهور", "Standard"), "school1": ("الحنفي", "Hanafi"),
    "adjust": ("تعديل بالدقائق", "Manual adjustment (min)"),
    "language": ("اللغة", "Language"), "theme": ("السمة", "Theme"),
    "dark": ("داكن", "Dark"), "light": ("فاتح", "Light"), "auto": ("تلقائي", "Auto"),
    "refresh": ("تحديث المواقيت", "Refresh times"),
    "refreshed": ("تم تحديث المواقيت", "Prayer times updated"),
    "refresh_fail": ("تعذّر الاتصال — تم استخدام الحساب المحلي", "Offline — using local calculation"),
    "autostart": ("التشغيل مع بدء Windows", "Launch at Windows startup"),
    "tray": ("الإبقاء في شريط النظام عند الإغلاق", "Keep running in tray when closed"),
    "show": ("إظهار", "Show"), "quit": ("إنهاء", "Quit"),
    # notification
    "time_for": ("حان وقت صلاة", "It's time for"), "in_min": ("بعد {n} دقيقة", "In {n} minutes"),
    "stop": ("إيقاف", "Stop"), "snooze": ("غفوة", "Snooze"), "open_app": ("فتح التطبيق", "Open app"),
    "snooze_n": ("غفوة {n} د", "Snooze {n}m"),
    # onboarding
    "ob_welcome": ("أهلاً بك", "Welcome"), "ob_pick_lang": ("اختر لغتك", "Choose your language"),
    "ob_how_loc": ("إزاي تحب نحدد موقعك؟", "How should we find your location?"),
    "ob_auto": ("تحديد تلقائي", "Automatic"), "ob_auto_sub": ("GPS أو عنوان الإنترنت", "GPS or internet address"),
    "ob_manual": ("اختيار يدوي", "Manual"), "ob_manual_sub": ("اختر الدولة والمحافظة", "Pick country & region"),
    "ob_found": ("هل هذا موقعك؟", "Is this your location?"), "ob_yes": ("نعم، صحيح", "Yes, that's right"),
    "ob_edit": ("تعديل", "Edit"), "ob_failed": ("تعذّر تحديد الموقع تلقائياً، اختر يدوياً", "Couldn't detect automatically — please choose manually"),
    "ob_pick_loc": ("اختر موقعك", "Choose your location"),
    "ob_method": ("طريقة حساب المواقيت", "Calculation method"),
    "ob_method_sub": ("اخترنا لك الأنسب لدولتك — تقدر تغيّرها", "We picked the best fit for your country — you can change it"),
    "ob_theme": ("اختر المظهر", "Choose your look"),
    "ob_done": ("كل شيء جاهز", "You're all set"), "ob_done_sub": ("تقبّل الله طاعاتكم", "May your prayers be accepted"),
    "next": ("التالي", "Continue"), "back": ("رجوع", "Back"), "start": ("ابدأ", "Get started"),
    "empty_title": ("لا توجد مواقيت", "No prayer times"), "loading": ("جارٍ التحميل…", "Loading…"),
    "pre_enable": ("تذكير قبل الصلاة", "Reminder before prayer"),
    "pre_minutes": ("قبل الصلاة بـ", "Minutes before"),
    "snooze_default": ("مدة الغفوة الافتراضية", "Default snooze length"),
    "sound_enable": ("تشغيل الصوت", "Play sound"),
    "adhan_sound": ("صوت الأذان", "Adhan sound"), "pre_sound": ("صوت التذكير", "Reminder sound"),
    "default_sound": ("الجرس الافتراضي", "Default chime"), "reset": ("إعادة الضبط", "Reset"),
    "test_pre": ("تجربة التذكير", "Test reminder"),
    "themes": ("الثيمات", "Themes"), "theme_mode": ("الوضع", "Mode"), "palette": ("لوحة الألوان", "Color palette"),
    "theme_hint": ("اختر لوناً — يتغيّر التطبيق فوراً", "Pick a color — the app updates instantly"),
    "open_themes": ("فتح الثيمات", "Open themes"),
    "snooze_custom": ("غفوة مخصصة", "Custom snooze"), "snooze_ph": ("دقائق", "minutes"),
    "unit_h": ("ساعة", "HOURS"), "unit_m": ("دقيقة", "MINUTES"), "unit_s": ("ثانية", "SECONDS"),
    "until": ("المتبقي حتى {name}", "Time until {name}"),
    "pre_title": ("تذكير: {name} بعد {n} دقيقة", "{name} in {n} min"),
    "custom_color": ("اختر لونك المفضّل", "Pick your favourite color"),
    "detected": ("تم تحديد موقعك:", "Detected:"),
    "sec_prayers": ("تنبيه كل صلاة", "Per-prayer alerts"),
    "col_alarm": ("الأذان", "AlAdhan"), "col_remind": ("التذكير", "Reminder"),
    "alarm_off": ("تم إيقاف التنبيه {name}", "{name} alarm turned off"),
    "alarm_on": ("تم تفعيل تنبيه {name}", "{name} alarm turned on"),
    "prayers_hint": ("أوقف المنبّه أو التذكير لأي صلاة بشكل منفصل", "Turn the alarm or the reminder off for any single prayer"),
    "about": ("حول البرنامج", "About"),
    "about_kicker": ("عن البرنامج", "ABOUT THE APPLICATION"),
    "about_body": ("V-Mawaqeet هو تطبيق صُمم ليكون رفيقك اليومي في متابعة مواقيت الصلاة، من خلال عرض أوقات الصلوات وتذكيرك بحلول موعد كل صلاة، لتبقى على صلة دائمة بأوقات العبادة وسط انشغالات يومك.",
                   "V-Mawaqeet is designed to be your daily companion in keeping track of prayer times. It displays prayer times and reminds you when each prayer is due, helping you stay connected to worship throughout the day."),
    "about_dua": ("نسأل الله أن يصلح أحوالنا جميعًا، وأن يجعل الصلاة راحةً لقلوبنا، ونورًا لدروبنا، وأن يعيننا على ذكره وشكره وحسن عبادته.",
                  "We ask Allah to set right the affairs of us all, to make prayer a source of comfort for our hearts and a light on our path, and to help us remember Him, thank Him, and worship Him in the best manner."),
    "about_amen": ("آمين يا رب العالمين.", "آمين يا رب العالمين."),
    "about_dua2": ("نسأل الله أن يجعل هذا العمل خالصًا لوجهه، وأن ينفع به، وأن يكتب لنا ولكم الأجر.",
                   "We ask Allah to make this work sincere for His sake, to make it beneficial, and to grant us all its reward."),
    "developer": ("المطوّر", "DEVELOPER"),
    "made_with": ("طُوّر بمساعدة Claude.", "Developed with the assistance of Claude."),
    "made_care": ("صُنع بعناية، ليكون تذكيرًا يوميًا ذا معنى.", "Made with care, for a meaningful daily reminder."),
    "version": ("الإصدار", "Version"), "close": ("إغلاق", "Close"),
    "copied": ("تم النسخ", "Copied"), "send_mail": ("إرسال بريد", "Send email"), "copy": ("نسخ", "Copy"),
    "iqama": ("الإقامة", "Iqama"), "iqama_title": ("حان وقت الإقامة", "Iqama time"),
    "iqama_sound": ("صوت الإقامة", "Iqama sound"),
    "col_before": ("قبل الأذان (د)", "before (min)"), "col_after": ("بعد الأذان (د)", "after (min)"),
    "prayers_hint2": ("حدّد لكل صلاة: المنبّه، والتذكير قبل الأذان بعدد دقائق تكتبه، والإقامة بعد الأذان بعدد دقائق تكتبه.",
                      "For each prayer choose: the alarm, a reminder before the adhan, and an iqama alert after it — each with its own minutes."),
    "tray_open": ("فتح V-Mawaqeet", "Open V-Mawaqeet"), "today_times": ("مواقيت اليوم", "Today's times"),
    "tray_alerts_on": ("التنبيهات: مفعّلة", "Alerts: on"), "tray_alerts_off": ("التنبيهات: مكتومة", "Alerts: muted"),
    "tray_alerts": ("تفعيل التنبيهات", "Alerts enabled"), "cancel_snooze": ("إلغاء الغفوة", "Cancel snooze"),
    "to_light": ("التبديل إلى الوضع الفاتح", "Switch to light mode"), "to_dark": ("التبديل إلى الوضع الداكن", "Switch to dark mode"),
    "switch_lang": ("English", "العربية"),
    "tray_next": ("{name} {time}  ·  متبقي {left}", "{name} {time}  ·  {left} left"),
    "hijri": ("هـ", "AH"),
}
DAYS = (("الإثنين", "الثلاثاء", "الأربعاء", "الخميس", "الجمعة", "السبت", "الأحد"),
        ("Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"))
MONTHS = (("يناير", "فبراير", "مارس", "أبريل", "مايو", "يونيو", "يوليو", "أغسطس", "سبتمبر", "أكتوبر", "نوفمبر", "ديسمبر"),
          ("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"))
HIJRI_MONTHS = (("محرم", "صفر", "ربيع الأول", "ربيع الآخر", "جمادى الأولى", "جمادى الآخرة", "رجب", "شعبان", "رمضان", "شوال", "ذو القعدة", "ذو الحجة"),
                ("Muharram", "Safar", "Rabi' I", "Rabi' II", "Jumada I", "Jumada II", "Rajab", "Sha'ban", "Ramadan", "Shawwal", "Dhu al-Qi'dah", "Dhu al-Hijjah"))


_friday = False


def set_friday(on: bool) -> None:
    """On Fridays the Dhuhr slot is shown as Jumu'ah (set from the selected place's local date)."""
    global _friday
    _friday = bool(on)


def is_friday() -> bool:
    return _friday


def set_language(lang: str) -> None:
    global _lang
    _lang = "en" if lang == "en" else "ar"


def lang() -> str:
    return _lang


def is_rtl() -> bool:
    return _lang == "ar"


def t(key: str, **kw) -> str:
    if key == "dhuhr" and _friday:
        key = "jumuah"
    v = S.get(key)
    s = (v[0] if _lang == "ar" else v[1]) if v else key
    return s.format(**kw) if kw else s


def tp(key: str) -> str:
    """Plain name in the UI language — never swapped for Jumu'ah (used where you configure the prayer itself)."""
    v = S.get(key)
    return (v[0] if _lang == "ar" else v[1]) if v else key


def tl(key: str, language: str) -> str:
    if key == "dhuhr" and _friday:
        key = "jumuah"
    v = S[key]
    return v[0] if language == "ar" else v[1]


def fmt_date(d, hijri: tuple[int, int, int] | None = None) -> str:
    i = 0 if _lang == "ar" else 1
    s = f"{DAYS[i][d.weekday()]}، {d.day} {MONTHS[i][d.month - 1]} {d.year}" if i == 0 else \
        f"{DAYS[i][d.weekday()]}, {d.day} {MONTHS[i][d.month - 1]} {d.year}"
    if hijri:
        hd, hm, hy = hijri
        s += f"  ·  {hd} {HIJRI_MONTHS[i][hm - 1]} {hy} {t('hijri')}"
    return s
