#include "hijri.h"

namespace vm {

HijriDate to_hijri(const QDate& d) {
    int y = d.year(), m = d.month(), dd = d.day();
    int a = (14 - m) / 12;
    int yy = y + 4800 - a;
    int mm = m + 12 * a - 3;
    int jd = dd + (153 * mm + 2) / 5 + 365 * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
    int l = jd - 1948440 + 10632;
    int n = (l - 1) / 10631;
    l = l - 10631 * n + 354;
    int j = ((10985 - l) / 5316) * ((50 * l) / 17719) + (l / 5670) * ((43 * l) / 15238);
    l = l - ((30 - j) / 15) * ((17719 * j) / 50) - (j / 16) * ((15238 * j) / 43) + 29;
    int month = (24 * l) / 709;
    int day = l - (709 * month) / 24;
    int year = 30 * n + j - 30;
    return HijriDate(day, month, year);
}

} // namespace vm
