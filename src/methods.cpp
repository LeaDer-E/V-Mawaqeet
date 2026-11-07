#include "methods.h"
#include <QVector>

namespace vm::methods {

static const QVector<Method>& all_methods() {
    static const QVector<Method> m = {
        {QStringLiteral("egypt"), 5, QStringLiteral("الهيئة المصرية العامة للمساحة"), QStringLiteral("Egyptian General Authority of Survey"), 19.5, 17.5, 0, 0.0},
        {QStringLiteral("ummalqura"), 4, QStringLiteral("جامعة أم القرى، مكة المكرمة"), QStringLiteral("Umm Al-Qura University, Makkah"), 18.5, 0.0, 90, 0.0},
        {QStringLiteral("mwl"), 3, QStringLiteral("رابطة العالم الإسلامي"), QStringLiteral("Muslim World League"), 18.0, 17.0, 0, 0.0},
        {QStringLiteral("isna"), 2, QStringLiteral("أمريكا الشمالية (ISNA)"), QStringLiteral("Islamic Society of North America"), 15.0, 15.0, 0, 0.0},
        {QStringLiteral("karachi"), 1, QStringLiteral("جامعة العلوم الإسلامية، كراتشي"), QStringLiteral("Univ. of Islamic Sciences, Karachi"), 18.0, 18.0, 0, 0.0},
        {QStringLiteral("kuwait"), 9, QStringLiteral("الكويت"), QStringLiteral("Kuwait"), 18.0, 17.5, 0, 0.0},
        {QStringLiteral("qatar"), 10, QStringLiteral("قطر"), QStringLiteral("Qatar"), 18.0, 0.0, 90, 0.0},
        {QStringLiteral("gulf"), 8, QStringLiteral("منطقة الخليج"), QStringLiteral("Gulf Region"), 19.5, 0.0, 90, 0.0},
        {QStringLiteral("dubai"), 16, QStringLiteral("دبي"), QStringLiteral("Dubai"), 18.2, 18.2, 0, 0.0},
        {QStringLiteral("jordan"), 23, QStringLiteral("وزارة الأوقاف الأردنية"), QStringLiteral("Jordan Ministry of Awqaf"), 18.0, 18.0, 0, 0.0},
        {QStringLiteral("turkey"), 13, QStringLiteral("رئاسة الشؤون الدينية، تركيا (ديانت)"), QStringLiteral("Diyanet İşleri Başkanlığı, Turkey"), 18.0, 17.0, 0, 0.0},
        {QStringLiteral("tehran"), 7, QStringLiteral("معهد الجيوفيزياء، جامعة طهران"), QStringLiteral("Institute of Geophysics, Univ. of Tehran"), 17.7, 14.0, 0, 4.5},
        {QStringLiteral("jafari"), 0, QStringLiteral("الشيعة الإثنا عشرية (ليفا، قم)"), QStringLiteral("Shia Ithna-Ashari (Leva, Qum)"), 16.0, 14.0, 0, 4.0},
        {QStringLiteral("singapore"), 11, QStringLiteral("سنغافورة (MUIS)"), QStringLiteral("Singapore (MUIS)"), 20.0, 18.0, 0, 0.0},
        {QStringLiteral("malaysia"), 17, QStringLiteral("ماليزيا (JAKIM)"), QStringLiteral("Malaysia (JAKIM)"), 20.0, 18.0, 0, 0.0},
        {QStringLiteral("indonesia"), 20, QStringLiteral("إندونيسيا (KEMENAG)"), QStringLiteral("Indonesia (Kemenag)"), 20.0, 18.0, 0, 0.0},
        {QStringLiteral("morocco"), 21, QStringLiteral("المغرب"), QStringLiteral("Morocco"), 19.0, 17.0, 0, 0.0},
        {QStringLiteral("algeria"), 19, QStringLiteral("الجزائر"), QStringLiteral("Algeria"), 18.0, 17.0, 0, 0.0},
        {QStringLiteral("tunisia"), 18, QStringLiteral("تونس"), QStringLiteral("Tunisia"), 18.0, 18.0, 0, 0.0},
        {QStringLiteral("france"), 12, QStringLiteral("اتحاد المنظمات الإسلامية بفرنسا"), QStringLiteral("Union of Islamic Organisations of France"), 12.0, 12.0, 0, 0.0},
        {QStringLiteral("russia"), 14, QStringLiteral("الإدارة الدينية لمسلمي روسيا"), QStringLiteral("Spiritual Administration of Muslims of Russia"), 16.0, 15.0, 0, 0.0},
        {QStringLiteral("portugal"), 22, QStringLiteral("البرتغال"), QStringLiteral("Portugal"), 18.0, 0.0, 77, 0.0},
        {QStringLiteral("moonsighting"), 15, QStringLiteral("لجنة رؤية الهلال"), QStringLiteral("Moonsighting Committee Worldwide"), 18.0, 18.0, 0, 0.0},
    };
    return m;
}

const QMap<QString, Method>& METHODS() {
    static QMap<QString, Method> map = []() {
        QMap<QString, Method> out;
        for (const auto& item : all_methods()) {
            out.insert(item.key, item);
        }
        return out;
    }();
    return map;
}

const QStringList& ORDER() {
    static QStringList ord = []() {
        QStringList out;
        for (const auto& item : all_methods()) {
            out.append(item.key);
        }
        return out;
    }();
    return ord;
}

QString default_for(const QString& country_code) {
    static const QMap<QString, QString> defaults = {
        {QStringLiteral("EG"), QStringLiteral("egypt")},
        {QStringLiteral("SD"), QStringLiteral("egypt")},
        {QStringLiteral("SS"), QStringLiteral("egypt")},
        {QStringLiteral("SA"), QStringLiteral("ummalqura")},
        {QStringLiteral("AE"), QStringLiteral("dubai")},
        {QStringLiteral("KW"), QStringLiteral("kuwait")},
        {QStringLiteral("QA"), QStringLiteral("qatar")},
        {QStringLiteral("BH"), QStringLiteral("gulf")},
        {QStringLiteral("OM"), QStringLiteral("gulf")},
        {QStringLiteral("YE"), QStringLiteral("ummalqura")},
        {QStringLiteral("JO"), QStringLiteral("jordan")},
        {QStringLiteral("PS"), QStringLiteral("jordan")},
        {QStringLiteral("LB"), QStringLiteral("mwl")},
        {QStringLiteral("SY"), QStringLiteral("mwl")},
        {QStringLiteral("IQ"), QStringLiteral("mwl")},
        {QStringLiteral("IR"), QStringLiteral("tehran")},
        {QStringLiteral("TR"), QStringLiteral("turkey")},
        {QStringLiteral("AZ"), QStringLiteral("turkey")},
        {QStringLiteral("PK"), QStringLiteral("karachi")},
        {QStringLiteral("IN"), QStringLiteral("karachi")},
        {QStringLiteral("BD"), QStringLiteral("karachi")},
        {QStringLiteral("AF"), QStringLiteral("karachi")},
        {QStringLiteral("LK"), QStringLiteral("karachi")},
        {QStringLiteral("MY"), QStringLiteral("malaysia")},
        {QStringLiteral("BN"), QStringLiteral("malaysia")},
        {QStringLiteral("SG"), QStringLiteral("singapore")},
        {QStringLiteral("ID"), QStringLiteral("indonesia")},
        {QStringLiteral("MA"), QStringLiteral("morocco")},
        {QStringLiteral("DZ"), QStringLiteral("algeria")},
        {QStringLiteral("TN"), QStringLiteral("tunisia")},
        {QStringLiteral("LY"), QStringLiteral("egypt")},
        {QStringLiteral("MR"), QStringLiteral("mwl")},
        {QStringLiteral("FR"), QStringLiteral("france")},
        {QStringLiteral("PT"), QStringLiteral("portugal")},
        {QStringLiteral("RU"), QStringLiteral("russia")},
        {QStringLiteral("KZ"), QStringLiteral("russia")},
        {QStringLiteral("US"), QStringLiteral("isna")},
        {QStringLiteral("CA"), QStringLiteral("isna")},
        {QStringLiteral("MX"), QStringLiteral("isna")},
        {QStringLiteral("GB"), QStringLiteral("moonsighting")}
    };
    return defaults.value(country_code.toUpper(), QStringLiteral("mwl"));
}

Method resolve(const QString& key, const QString& country_code) {
    QString k = key;
    if (k == QStringLiteral("auto") || !METHODS().contains(k)) {
        k = default_for(country_code);
    }
    return METHODS().value(k);
}

} // namespace vm::methods
