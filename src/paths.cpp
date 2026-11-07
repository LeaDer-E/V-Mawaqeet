#include "paths.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>

namespace vm::paths {

QDir resource_root() {
    QDir exeDir(QCoreApplication::applicationDirPath());
    if (exeDir.exists(QStringLiteral("assets"))) {
        return exeDir;
    }
    QDir parentDir = exeDir;
    if (parentDir.cdUp() && parentDir.exists(QStringLiteral("assets"))) {
        return parentDir;
    }
    QDir parent2Dir = parentDir;
    if (parent2Dir.cdUp() && parent2Dir.exists(QStringLiteral("assets"))) {
        return parent2Dir;
    }
#ifdef Q_OS_LINUX
    QDir shareDir(QStringLiteral("/usr/share/V-Mawaqeet"));
    if (shareDir.exists(QStringLiteral("assets"))) {
        return shareDir;
    }
    QDir localOpt(QDir::homePath() + QStringLiteral("/.local/opt/V-Mawaqeet"));
    if (localOpt.exists(QStringLiteral("assets"))) {
        return localOpt;
    }
#endif
    return QDir::current();
}

QString asset(const QStringList& parts) {
    QString p = resource_root().filePath(QStringLiteral("assets"));
    for (const QString& part : parts) {
        p = QDir(p).filePath(part);
    }
    return p;
}

QDir user_dir() {
    QString base;
#if defined(Q_OS_WIN)
    base = qEnvironmentVariable("APPDATA");
    if (base.isEmpty()) {
        base = QDir(QDir::homePath()).filePath(QStringLiteral("AppData/Roaming"));
    }
#elif defined(Q_OS_MACOS)
    base = QDir(QDir::homePath()).filePath(QStringLiteral("Library/Application Support"));
#else
    base = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (base.isEmpty()) {
        base = QDir(QDir::homePath()).filePath(QStringLiteral(".config"));
    }
#endif
    QDir d(QDir(base).filePath(QStringLiteral("V-Mawaqeet")));
    if (!d.exists()) {
        d.mkpath(QStringLiteral("."));
    }
    return d;
}

QDir cache_dir() {
    QDir d(user_dir().filePath(QStringLiteral("cache")));
    if (!d.exists()) {
        d.mkpath(QStringLiteral("."));
    }
    return d;
}

} // namespace vm::paths
