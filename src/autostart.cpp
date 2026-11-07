#include "autostart.h"
#include "paths.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

namespace vm::autostart {

bool supported() {
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    return true;
#else
    return false;
#endif
}

bool set_enabled(bool enabled) {
#if defined(Q_OS_WIN)
    QSettings reg(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                  QSettings::NativeFormat);
    if (enabled) {
        QString cmd = QStringLiteral("\"%1\" --minimized")
                          .arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
        reg.setValue(QStringLiteral("V-Mawaqeet"), cmd);
    } else {
        reg.remove(QStringLiteral("V-Mawaqeet"));
    }
    return reg.status() == QSettings::NoError;
#elif defined(Q_OS_LINUX)
    QString configHome = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (configHome.isEmpty()) {
        configHome = QDir(QDir::homePath()).filePath(QStringLiteral(".config"));
    }
    QDir autoDir(QDir(configHome).filePath(QStringLiteral("autostart")));
    QString desktopFile = autoDir.filePath(QStringLiteral("V-Mawaqeet.desktop"));
    if (!enabled) {
        if (QFile::exists(desktopFile)) {
            return QFile::remove(desktopFile);
        }
        return true;
    }
    if (!autoDir.exists()) {
        autoDir.mkpath(QStringLiteral("."));
    }
    QFile f(desktopFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    QString exe = QCoreApplication::applicationFilePath();
    QString iconPath = paths::asset(QStringLiteral("icon.png"));
    QString content = QStringLiteral(
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=V-Mawaqeet\n"
        "Comment=Prayer Times & Smart Reminders\n"
        "Exec=\"%1\" --minimized\n"
        "Icon=%2\n"
        "Terminal=false\n"
        "Categories=Utility;\n"
        "StartupNotify=false\n"
    ).arg(exe, iconPath);
    f.write(content.toUtf8());
    return true;
#else
    Q_UNUSED(enabled);
    return false;
#endif
}

} // namespace vm::autostart
