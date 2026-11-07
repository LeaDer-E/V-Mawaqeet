#pragma once
#include "hijri.h"
#include <QDate>
#include <QMap>
#include <QString>
#include <utility>

namespace vm::i18n {

void set_friday(bool on);
bool is_friday();
void set_language(const QString& lang);
QString lang();
bool is_rtl();
QString t(const QString& key, const QMap<QString, QString>& kw = {});
QString tp(const QString& key);
QString tl(const QString& key, const QString& language);
QString fmt_date(const QDate& d, const HijriDate& hijri = {});

} // namespace vm::i18n
