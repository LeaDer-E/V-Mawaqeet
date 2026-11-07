#include "i18n.h"
#include <QStringList>

namespace vm::i18n {

static QString g_lang = QStringLiteral("ar");
static bool g_friday = false;

static const QMap<QString, std::pair<QString, QString>>& strings() {
    static const QMap<QString, std::pair<QString, QString>> S = {
        {QStringLiteral("app"), {QStringLiteral("مواقيت"), QStringLiteral("V-Mawaqeet")}},
        {QStringLiteral("home"), {QStringLiteral("الرئيسية"), QStringLiteral("Home")}},
        {QStringLiteral("settings"), {QStringLiteral("الإعدادات"), QStringLiteral("Settings")}},
        {QStringLiteral("next_prayer"), {QStringLiteral("الصلاة القادمة"), QStringLiteral("Next prayer")}},
        {QStringLiteral("remaining"), {QStringLiteral("الوقت المتبقي"), QStringLiteral("Time remaining")}},
        {QStringLiteral("upcoming"), {QStringLiteral("القادمة"), QStringLiteral("NEXT")}},
        {QStringLiteral("fajr"), {QStringLiteral("الفجر"), QStringLiteral("Fajr")}},
        {QStringLiteral("sunrise"), {QStringLiteral("الشروق"), QStringLiteral("Sunrise")}},
        {QStringLiteral("dhuhr"), {QStringLiteral("الظهر"), QStringLiteral("Dhuhr")}},
        {QStringLiteral("jumuah"), {QStringLiteral("الجمعة"), QStringLiteral("Jumu'ah")}},
        {QStringLiteral("asr"), {QStringLiteral("العصر"), QStringLiteral("Asr")}},
        {QStringLiteral("maghrib"), {QStringLiteral("المغرب"), QStringLiteral("Maghrib")}},
        {QStringLiteral("isha"), {QStringLiteral("العشاء"), QStringLiteral("Isha")}},
        {QStringLiteral("source"), {QStringLiteral("المصدر"), QStringLiteral("Source")}},
        {QStringLiteral("method"), {QStringLiteral("طريقة الحساب"), QStringLiteral("Calculation method")}},
        {QStringLiteral("snoozed"), {QStringLiteral("غفوة"), QStringLiteral("Snoozed")}},
        // settings
        {QStringLiteral("sec_location"), {QStringLiteral("الموقع"), QStringLiteral("Location")}},
        {QStringLiteral("sec_alerts"), {QStringLiteral("التنبيهات"), QStringLiteral("Notifications")}},
        {QStringLiteral("sec_sources"), {QStringLiteral("مصادر المواقيت"), QStringLiteral("Prayer-time sources")}},
        {QStringLiteral("sec_look"), {QStringLiteral("المظهر"), QStringLiteral("Appearance")}},
        {QStringLiteral("sec_calc"), {QStringLiteral("الحساب والتعديل اليدوي"), QStringLiteral("Calculation & fine-tuning")}},
        {QStringLiteral("sec_system"), {QStringLiteral("النظام"), QStringLiteral("System")}},
        {QStringLiteral("auto_loc"), {QStringLiteral("تحديد الموقع تلقائياً (GPS / IP)"), QStringLiteral("Detect my location (GPS / IP)")}},
        {QStringLiteral("detect_now"), {QStringLiteral("حدّد الآن"), QStringLiteral("Detect now")}},
        {QStringLiteral("detecting"), {QStringLiteral("جارٍ تحديد موقعك…"), QStringLiteral("Finding your location…")}},
        {QStringLiteral("country"), {QStringLiteral("الدولة"), QStringLiteral("Country")}},
        {QStringLiteral("region"), {QStringLiteral("المحافظة / الولاية"), QStringLiteral("Region / State")}},
        {QStringLiteral("search"), {QStringLiteral("ابحث…"), QStringLiteral("Search…")}},
        {QStringLiteral("no_results"), {QStringLiteral("لا توجد نتائج"), QStringLiteral("No results")}},
        {QStringLiteral("enable_alerts"), {QStringLiteral("تفعيل تنبيهات الأذان"), QStringLiteral("Enable adhan alerts")}},
        {QStringLiteral("alert_sunrise"), {QStringLiteral("تنبيه وقت الشروق"), QStringLiteral("Alert at sunrise")}},
        {QStringLiteral("snooze_len"), {QStringLiteral("مدة الغفوة"), QStringLiteral("Snooze length")}},
        {QStringLiteral("minutes"), {QStringLiteral("دقيقة"), QStringLiteral("min")}},
        {QStringLiteral("pre_rem"), {QStringLiteral("تذكير قبل الصلاة"), QStringLiteral("Reminder before prayer")}},
        {QStringLiteral("off"), {QStringLiteral("إيقاف"), QStringLiteral("Off")}},
        {QStringLiteral("sound"), {QStringLiteral("صوت التنبيه"), QStringLiteral("Alert sound")}},
        {QStringLiteral("chime"), {QStringLiteral("جرس هادئ"), QStringLiteral("Soft chime")}},
        {QStringLiteral("custom"), {QStringLiteral("ملف خاص"), QStringLiteral("Custom file")}},
        {QStringLiteral("silent"), {QStringLiteral("صامت"), QStringLiteral("Silent")}},
        {QStringLiteral("choose_file"), {QStringLiteral("اختيار ملف الأذان…"), QStringLiteral("Choose adhan file…")}},
        {QStringLiteral("preview"), {QStringLiteral("استماع"), QStringLiteral("Preview")}},
        {QStringLiteral("test_notif"), {QStringLiteral("تجربة الإشعار"), QStringLiteral("Test notification")}},
        {QStringLiteral("primary_src"), {QStringLiteral("وضع المصدر"), QStringLiteral("Source mode")}},
        {QStringLiteral("src_auto"), {QStringLiteral("تلقائي (إنترنت ← حساب محلي)"), QStringLiteral("Auto (online → local)")}},
        {QStringLiteral("src_offline"), {QStringLiteral("حساب محلي فقط"), QStringLiteral("Local calculation only")}},
        {QStringLiteral("src_local"), {QStringLiteral("حساب محلي (Offline)"), QStringLiteral("Local calculation (offline)")}},
        {QStringLiteral("priority"), {QStringLiteral("ترتيب الأولوية"), QStringLiteral("Fallback order")}},
        {QStringLiteral("connected"), {QStringLiteral("متصل"), QStringLiteral("Connected")}},
        {QStringLiteral("failed"), {QStringLiteral("فشل"), QStringLiteral("Failed")}},
        {QStringLiteral("offline_ready"), {QStringLiteral("جاهز دائماً"), QStringLiteral("Always ready")}},
        {QStringLiteral("cached"), {QStringLiteral("من الذاكرة"), QStringLiteral("Cached")}},
        {QStringLiteral("idle"), {QStringLiteral("لم يُستخدم"), QStringLiteral("Unused")}},
        {QStringLiteral("school"), {QStringLiteral("المذهب (العصر)"), QStringLiteral("Asr school")}},
        {QStringLiteral("school0"), {QStringLiteral("الجمهور"), QStringLiteral("Standard")}},
        {QStringLiteral("school1"), {QStringLiteral("الحنفي"), QStringLiteral("Hanafi")}},
        {QStringLiteral("adjust"), {QStringLiteral("تعديل بالدقائق"), QStringLiteral("Manual adjustment (min)")}},
        {QStringLiteral("language"), {QStringLiteral("اللغة"), QStringLiteral("Language")}},
        {QStringLiteral("theme"), {QStringLiteral("السمة"), QStringLiteral("Theme")}},
        {QStringLiteral("dark"), {QStringLiteral("داكن"), QStringLiteral("Dark")}},
        {QStringLiteral("light"), {QStringLiteral("فاتح"), QStringLiteral("Light")}},
        {QStringLiteral("auto"), {QStringLiteral("تلقائي"), QStringLiteral("Auto")}},
        {QStringLiteral("refresh"), {QStringLiteral("تحديث المواقيت"), QStringLiteral("Refresh times")}},
        {QStringLiteral("refreshed"), {QStringLiteral("تم تحديث المواقيت"), QStringLiteral("Prayer times updated")}},
        {QStringLiteral("refresh_fail"), {QStringLiteral("تعذّر الاتصال — تم استخدام الحساب المحلي"), QStringLiteral("Offline — using local calculation")}},
        {QStringLiteral("autostart"), {QStringLiteral("التشغيل مع بدء النظام"), QStringLiteral("Launch at system startup")}},
        {QStringLiteral("tray"), {QStringLiteral("الإبقاء في شريط النظام عند الإغلاق"), QStringLiteral("Keep running in tray when closed")}},
        {QStringLiteral("show"), {QStringLiteral("إظهار"), QStringLiteral("Show")}},
        {QStringLiteral("quit"), {QStringLiteral("إنهاء"), QStringLiteral("Quit")}},
        // notification
        {QStringLiteral("time_for"), {QStringLiteral("حان وقت صلاة"), QStringLiteral("It's time for")}},
        {QStringLiteral("in_min"), {QStringLiteral("بعد {n} دقيقة"), QStringLiteral("In {n} minutes")}},
        {QStringLiteral("stop"), {QStringLiteral("إيقاف"), QStringLiteral("Stop")}},
        {QStringLiteral("snooze"), {QStringLiteral("غفوة"), QStringLiteral("Snooze")}},
        {QStringLiteral("open_app"), {QStringLiteral("فتح التطبيق"), QStringLiteral("Open app")}},
        {QStringLiteral("snooze_n"), {QStringLiteral("غفوة {n} د"), QStringLiteral("Snooze {n}m")}},
        // onboarding
        {QStringLiteral("ob_welcome"), {QStringLiteral("أهلاً بك"), QStringLiteral("Welcome")}},
        {QStringLiteral("ob_pick_lang"), {QStringLiteral("اختر لغتك"), QStringLiteral("Choose your language")}},
        {QStringLiteral("ob_how_loc"), {QStringLiteral("إزاي تحب نحدد موقعك؟"), QStringLiteral("How should we find your location?")}},
        {QStringLiteral("ob_auto"), {QStringLiteral("تحديد تلقائي"), QStringLiteral("Automatic")}},
        {QStringLiteral("ob_auto_sub"), {QStringLiteral("GPS أو عنوان الإنترنت"), QStringLiteral("GPS or internet address")}},
        {QStringLiteral("ob_manual"), {QStringLiteral("اختيار يدوي"), QStringLiteral("Manual")}},
        {QStringLiteral("ob_manual_sub"), {QStringLiteral("اختر الدولة والمحافظة"), QStringLiteral("Pick country & region")}},
        {QStringLiteral("ob_found"), {QStringLiteral("هل هذا موقعك؟"), QStringLiteral("Is this your location?")}},
        {QStringLiteral("ob_yes"), {QStringLiteral("نعم، صحيح"), QStringLiteral("Yes, that's right")}},
        {QStringLiteral("ob_edit"), {QStringLiteral("تعديل"), QStringLiteral("Edit")}},
        {QStringLiteral("ob_failed"), {QStringLiteral("تعذّر تحديد الموقع تلقائياً، اختر يدوياً"), QStringLiteral("Couldn't detect automatically — please choose manually")}},
        {QStringLiteral("ob_pick_loc"), {QStringLiteral("اختر موقعك"), QStringLiteral("Choose your location")}},
        {QStringLiteral("ob_method"), {QStringLiteral("طريقة حساب المواقيت"), QStringLiteral("Calculation method")}},
        {QStringLiteral("ob_method_sub"), {QStringLiteral("اخترنا لك الأنسب لدولتك — تقدر تغيّرها"), QStringLiteral("We picked the best fit for your country — you can change it")}},
        {QStringLiteral("ob_theme"), {QStringLiteral("اختر المظهر"), QStringLiteral("Choose your look")}},
        {QStringLiteral("ob_done"), {QStringLiteral("كل شيء جاهز"), QStringLiteral("You're all set")}},
        {QStringLiteral("ob_done_sub"), {QStringLiteral("تقبّل الله طاعاتكم"), QStringLiteral("May your prayers be accepted")}},
        {QStringLiteral("next"), {QStringLiteral("التالي"), QStringLiteral("Continue")}},
        {QStringLiteral("back"), {QStringLiteral("رجوع"), QStringLiteral("Back")}},
        {QStringLiteral("start"), {QStringLiteral("ابدأ"), QStringLiteral("Get started")}},
        {QStringLiteral("empty_title"), {QStringLiteral("لا توجد مواقيت"), QStringLiteral("No prayer times")}},
        {QStringLiteral("loading"), {QStringLiteral("جارٍ التحميل…"), QStringLiteral("Loading…")}},
        {QStringLiteral("pre_enable"), {QStringLiteral("تذكير قبل الصلاة"), QStringLiteral("Reminder before prayer")}},
        {QStringLiteral("pre_minutes"), {QStringLiteral("قبل الصلاة بـ"), QStringLiteral("Minutes before")}},
        {QStringLiteral("snooze_default"), {QStringLiteral("مدة الغفوة الافتراضية"), QStringLiteral("Default snooze length")}},
        {QStringLiteral("sound_enable"), {QStringLiteral("تشغيل الصوت"), QStringLiteral("Play sound")}},
        {QStringLiteral("adhan_sound"), {QStringLiteral("صوت الأذان"), QStringLiteral("Adhan sound")}},
        {QStringLiteral("pre_sound"), {QStringLiteral("صوت التذكير"), QStringLiteral("Reminder sound")}},
        {QStringLiteral("default_sound"), {QStringLiteral("الجرس الافتراضي"), QStringLiteral("Default chime")}},
        {QStringLiteral("reset"), {QStringLiteral("إعادة الضبط"), QStringLiteral("Reset")}},
        {QStringLiteral("test_pre"), {QStringLiteral("تجربة التذكير"), QStringLiteral("Test reminder")}},
        {QStringLiteral("themes"), {QStringLiteral("الثيمات"), QStringLiteral("Themes")}},
        {QStringLiteral("theme_mode"), {QStringLiteral("الوضع"), QStringLiteral("Mode")}},
        {QStringLiteral("palette"), {QStringLiteral("لوحة الألوان"), QStringLiteral("Color palette")}},
        {QStringLiteral("theme_hint"), {QStringLiteral("اختر لوناً — يتغيّر التطبيق فوراً"), QStringLiteral("Pick a color — the app updates instantly")}},
        {QStringLiteral("open_themes"), {QStringLiteral("فتح الثيمات"), QStringLiteral("Open themes")}},
        {QStringLiteral("snooze_custom"), {QStringLiteral("غفوة مخصصة"), QStringLiteral("Custom snooze")}},
        {QStringLiteral("snooze_ph"), {QStringLiteral("دقائق"), QStringLiteral("minutes")}},
        {QStringLiteral("unit_h"), {QStringLiteral("ساعة"), QStringLiteral("HOURS")}},
        {QStringLiteral("unit_m"), {QStringLiteral("دقيقة"), QStringLiteral("MINUTES")}},
        {QStringLiteral("unit_s"), {QStringLiteral("ثانية"), QStringLiteral("SECONDS")}},
        {QStringLiteral("until"), {QStringLiteral("المتبقي حتى {name}"), QStringLiteral("Time until {name}")}},
        {QStringLiteral("pre_title"), {QStringLiteral("تذكير: {name} بعد {n} دقيقة"), QStringLiteral("{name} in {n} min")}},
        {QStringLiteral("custom_color"), {QStringLiteral("اختر لونك المفضّل"), QStringLiteral("Pick your favourite color")}},
        {QStringLiteral("detected"), {QStringLiteral("تم تحديد موقعك:"), QStringLiteral("Detected:")}},
        {QStringLiteral("sec_prayers"), {QStringLiteral("تنبيه كل صلاة"), QStringLiteral("Per-prayer alerts")}},
        {QStringLiteral("col_alarm"), {QStringLiteral("الأذان"), QStringLiteral("AlAdhan")}},
        {QStringLiteral("col_remind"), {QStringLiteral("التذكير"), QStringLiteral("Reminder")}},
        {QStringLiteral("alarm_off"), {QStringLiteral("تم إيقاف التنبيه {name}"), QStringLiteral("{name} alarm turned off")}},
        {QStringLiteral("alarm_on"), {QStringLiteral("تم تفعيل تنبيه {name}"), QStringLiteral("{name} alarm turned on")}},
        {QStringLiteral("prayers_hint"), {QStringLiteral("أوقف المنبّه أو التذكير لأي صلاة بشكل منفصل"), QStringLiteral("Turn the alarm or the reminder off for any single prayer")}},
        {QStringLiteral("about"), {QStringLiteral("حول البرنامج"), QStringLiteral("About")}},
        {QStringLiteral("about_kicker"), {QStringLiteral("عن البرنامج"), QStringLiteral("ABOUT THE APPLICATION")}},
        {QStringLiteral("about_body"), {
            QStringLiteral("V-Mawaqeet هو تطبيق صُمم ليكون رفيقك اليومي في متابعة مواقيت الصلاة، من خلال عرض أوقات الصلوات وتذكيرك بحلول موعد كل صلاة، لتبقى على صلة دائمة بأوقات العبادة وسط انشغالات يومك."),
            QStringLiteral("V-Mawaqeet is designed to be your daily companion in keeping track of prayer times. It displays prayer times and reminds you when each prayer is due, helping you stay connected to worship throughout the day.")
        }},
        {QStringLiteral("about_dua"), {
            QStringLiteral("نسأل الله أن يصلح أحوالنا جميعًا، وأن يجعل الصلاة راحةً لقلوبنا، ونورًا لدروبنا، وأن يعيننا على ذكره وشكره وحسن عبادته."),
            QStringLiteral("We ask Allah to set right the affairs of us all, to make prayer a source of comfort for our hearts and a light on our path, and to help us remember Him, thank Him, and worship Him in the best manner.")
        }},
        {QStringLiteral("about_amen"), {QStringLiteral("آمين يا رب العالمين."), QStringLiteral("آمين يا رب العالمين.")}},
        {QStringLiteral("about_dua2"), {
            QStringLiteral("نسأل الله أن يجعل هذا العمل خالصًا لوجهه، وأن ينفع به، وأن يكتب لنا ولكم الأجر."),
            QStringLiteral("We ask Allah to make this work sincere for His sake, to make it beneficial, and to grant us all its reward.")
        }},
        {QStringLiteral("developer"), {QStringLiteral("المطوّر"), QStringLiteral("DEVELOPER")}},
        {QStringLiteral("made_with"), {QStringLiteral("طُوّر بمساعدة Argon."), QStringLiteral("Developed with the assistance of Argon.")}},
        {QStringLiteral("made_care"), {QStringLiteral("صُنع بعناية، ليكون تذكيرًا يوميًا ذا معنى."), QStringLiteral("Made with care, for a meaningful daily reminder.")}},
        {QStringLiteral("version"), {QStringLiteral("الإصدار"), QStringLiteral("Version")}},
        {QStringLiteral("close"), {QStringLiteral("إغلاق"), QStringLiteral("Close")}},
        {QStringLiteral("copied"), {QStringLiteral("تم النسخ"), QStringLiteral("Copied")}},
        {QStringLiteral("send_mail"), {QStringLiteral("إرسال بريد"), QStringLiteral("Send email")}},
        {QStringLiteral("copy"), {QStringLiteral("نسخ"), QStringLiteral("Copy")}},
        {QStringLiteral("iqama"), {QStringLiteral("الإقامة"), QStringLiteral("Iqama")}},
        {QStringLiteral("iqama_title"), {QStringLiteral("حان وقت الإقامة"), QStringLiteral("Iqama time")}},
        {QStringLiteral("iqama_sound"), {QStringLiteral("صوت الإقامة"), QStringLiteral("Iqama sound")}},
        {QStringLiteral("col_before"), {QStringLiteral("قبل الأذان (د)"), QStringLiteral("before (min)")}},
        {QStringLiteral("col_after"), {QStringLiteral("بعد الأذان (د)"), QStringLiteral("after (min)")}},
        {QStringLiteral("prayers_hint2"), {
            QStringLiteral("حدّد لكل صلاة: المنبّه، والتذكير قبل الأذان بعدد دقائق تكتبه، والإقامة بعد الأذان بعدد دقائق تكتبه."),
            QStringLiteral("For each prayer choose: the alarm, a reminder before the adhan, and an iqama alert after it — each with its own minutes.")
        }},
        {QStringLiteral("tray_open"), {QStringLiteral("فتح V-Mawaqeet"), QStringLiteral("Open V-Mawaqeet")}},
        {QStringLiteral("today_times"), {QStringLiteral("مواقيت اليوم"), QStringLiteral("Today's times")}},
        {QStringLiteral("tray_alerts_on"), {QStringLiteral("التنبيهات: مفعّلة"), QStringLiteral("Alerts: on")}},
        {QStringLiteral("tray_alerts_off"), {QStringLiteral("التنبيهات: مكتومة"), QStringLiteral("Alerts: muted")}},
        {QStringLiteral("tray_alerts"), {QStringLiteral("تفعيل التنبيهات"), QStringLiteral("Alerts enabled")}},
        {QStringLiteral("cancel_snooze"), {QStringLiteral("إلغاء الغفوة"), QStringLiteral("Cancel snooze")}},
        {QStringLiteral("to_light"), {QStringLiteral("التبديل إلى الوضع الفاتح"), QStringLiteral("Switch to light mode")}},
        {QStringLiteral("to_dark"), {QStringLiteral("التبديل إلى الوضع الداكن"), QStringLiteral("Switch to dark mode")}},
        {QStringLiteral("switch_lang"), {QStringLiteral("English"), QStringLiteral("العربية")}},
        {QStringLiteral("tray_next"), {QStringLiteral("{name} {time}  ·  متبقي {left}"), QStringLiteral("{name} {time}  ·  {left} left")}},
        {QStringLiteral("hijri"), {QStringLiteral("هـ"), QStringLiteral("AH")}},
    };
    return S;
}

void set_friday(bool on) { g_friday = on; }
bool is_friday() { return g_friday; }
void set_language(const QString& lang) { g_lang = (lang == QStringLiteral("en")) ? QStringLiteral("en") : QStringLiteral("ar"); }
QString lang() { return g_lang; }
bool is_rtl() { return g_lang == QStringLiteral("ar"); }

QString t(const QString& key, const QMap<QString, QString>& kw) {
    QString k = (key == QStringLiteral("dhuhr") && g_friday) ? QStringLiteral("jumuah") : key;
    const auto& S = strings();
    auto it = S.find(k);
    QString s = (it != S.end()) ? ((g_lang == QStringLiteral("ar")) ? it->first : it->second) : k;
    for (auto kit = kw.begin(); kit != kw.end(); ++kit) {
        s.replace(QStringLiteral("{") + kit.key() + QStringLiteral("}"), kit.value());
    }
    return s;
}

QString tp(const QString& key) {
    const auto& S = strings();
    auto it = S.find(key);
    return (it != S.end()) ? ((g_lang == QStringLiteral("ar")) ? it->first : it->second) : key;
}

QString tl(const QString& key, const QString& language) {
    QString k = (key == QStringLiteral("dhuhr") && g_friday) ? QStringLiteral("jumuah") : key;
    const auto& S = strings();
    auto it = S.find(k);
    return (it != S.end()) ? ((language == QStringLiteral("ar")) ? it->first : it->second) : k;
}

QString fmt_date(const QDate& d, const HijriDate& hijri) {
    static const QStringList DAYS_AR = {
        QStringLiteral("الإثنين"), QStringLiteral("الثلاثاء"), QStringLiteral("الأربعاء"),
        QStringLiteral("الخميس"), QStringLiteral("الجمعة"), QStringLiteral("السبت"), QStringLiteral("الأحد")
    };
    static const QStringList DAYS_EN = {
        QStringLiteral("Monday"), QStringLiteral("Tuesday"), QStringLiteral("Wednesday"),
        QStringLiteral("Thursday"), QStringLiteral("Friday"), QStringLiteral("Saturday"), QStringLiteral("Sunday")
    };
    static const QStringList MONTHS_AR = {
        QStringLiteral("يناير"), QStringLiteral("فبراير"), QStringLiteral("مارس"), QStringLiteral("أبريل"),
        QStringLiteral("مايو"), QStringLiteral("يونيو"), QStringLiteral("يوليو"), QStringLiteral("أغسطس"),
        QStringLiteral("سبتمبر"), QStringLiteral("أكتوبر"), QStringLiteral("نوفمبر"), QStringLiteral("ديسمبر")
    };
    static const QStringList MONTHS_EN = {
        QStringLiteral("January"), QStringLiteral("February"), QStringLiteral("March"), QStringLiteral("April"),
        QStringLiteral("May"), QStringLiteral("June"), QStringLiteral("July"), QStringLiteral("August"),
        QStringLiteral("September"), QStringLiteral("October"), QStringLiteral("November"), QStringLiteral("December")
    };
    static const QStringList HIJRI_AR = {
        QStringLiteral("محرم"), QStringLiteral("صفر"), QStringLiteral("ربيع الأول"), QStringLiteral("ربيع الآخر"),
        QStringLiteral("جمادى الأولى"), QStringLiteral("جمادى الآخرة"), QStringLiteral("رجب"), QStringLiteral("شعبان"),
        QStringLiteral("رمضان"), QStringLiteral("شوال"), QStringLiteral("ذو القعدة"), QStringLiteral("ذو الحجة")
    };
    static const QStringList HIJRI_EN = {
        QStringLiteral("Muharram"), QStringLiteral("Safar"), QStringLiteral("Rabi' I"), QStringLiteral("Rabi' II"),
        QStringLiteral("Jumada I"), QStringLiteral("Jumada II"), QStringLiteral("Rajab"), QStringLiteral("Sha'ban"),
        QStringLiteral("Ramadan"), QStringLiteral("Shawwal"), QStringLiteral("Dhu al-Qi'dah"), QStringLiteral("Dhu al-Hijjah")
    };

    bool ar = (g_lang == QStringLiteral("ar"));
    int wd = std::max(1, std::min(7, d.dayOfWeek())) - 1;
    int mo = std::max(1, std::min(12, d.month())) - 1;
    QString dayName = ar ? DAYS_AR[wd] : DAYS_EN[wd];
    QString monthName = ar ? MONTHS_AR[mo] : MONTHS_EN[mo];
    QString s = ar ? QStringLiteral("%1، %2 %3 %4").arg(dayName).arg(d.day()).arg(monthName).arg(d.year())
                   : QStringLiteral("%1, %2 %3 %4").arg(dayName).arg(d.day()).arg(monthName).arg(d.year());
    if (hijri.valid) {
        int hm = std::max(1, std::min(12, hijri.month)) - 1;
        QString hMonthName = ar ? HIJRI_AR[hm] : HIJRI_EN[hm];
        s += QStringLiteral("  ·  %1 %2 %3 %4").arg(hijri.day).arg(hMonthName).arg(hijri.year).arg(t(QStringLiteral("hijri")));
    }
    return s;
}

} // namespace vm::i18n
