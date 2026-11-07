#pragma once
#include <QDir>
#include <QString>
#include <QStringList>

namespace vm::paths {

QDir resource_root();
QString asset(const QStringList& parts);
inline QString asset(const QString& p1) { return asset(QStringList{p1}); }
inline QString asset(const QString& p1, const QString& p2) { return asset(QStringList{p1, p2}); }
QDir user_dir();
QDir cache_dir();

} // namespace vm::paths
