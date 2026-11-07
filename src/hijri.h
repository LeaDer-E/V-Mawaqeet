#pragma once
#include <QDate>

namespace vm {

struct HijriDate {
    int day = 1;
    int month = 1;
    int year = 1446;
    bool valid = false;

    HijriDate() = default;
    HijriDate(int d, int m, int y) : day(d), month(m), year(y), valid(true) {}
};

HijriDate to_hijri(const QDate& d);

} // namespace vm
