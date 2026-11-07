#include "app_info.h"
#include "config.h"
#include "i18n.h"
#include "paths.h"
#include "ui/window.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QLockFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shobjidl.h>
#endif

namespace {

QFile g_logFile;
QMutex g_logMutex;

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    QMutexLocker locker(&g_logMutex);
    if (!g_logFile.isOpen()) return;
    QString level = QStringLiteral("INFO");
    switch (type) {
    case QtDebugMsg: level = QStringLiteral("DEBUG"); break;
    case QtInfoMsg: level = QStringLiteral("INFO"); break;
    case QtWarningMsg: level = QStringLiteral("WARNING"); break;
    case QtCriticalMsg: level = QStringLiteral("ERROR"); break;
    case QtFatalMsg: level = QStringLiteral("CRITICAL"); break;
    }
    QString ts = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss,zzz"));
    QString cat = (ctx.category && ctx.category[0]) ? QString::fromUtf8(ctx.category) : QStringLiteral("vm");
    QTextStream tsOut(&g_logFile);
    tsOut << ts << " " << level << " " << cat << ": " << msg << "\n";
    tsOut.flush();
}

} // namespace

int main(int argc, char* argv[]) {
    g_logFile.setFileName(vm::paths::user_dir().filePath(QStringLiteral("app.log")));
    if (g_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qInstallMessageHandler(logMessageHandler);
    }

#if defined(Q_OS_WIN)
    SetCurrentProcessExplicitAppUserModelID(L"VMawaqeet.App");
#endif

    QApplication app(argc, argv);
    app.setApplicationName(vm::APP_NAME);
    app.setApplicationVersion(vm::APP_VERSION);
    app.setQuitOnLastWindowClosed(false);

    QLockFile lock(vm::paths::user_dir().filePath(QStringLiteral("app.lock")));
    if (!lock.tryLock(100)) {
        return 0; // already running
    }

    QDir fontsDir(vm::paths::asset(QStringLiteral("fonts")));
    const QStringList ttfFiles = fontsDir.entryList({QStringLiteral("*.ttf")}, QDir::Files, QDir::Name);
    for (const QString& f : ttfFiles) {
        QFontDatabase::addApplicationFont(fontsDir.filePath(f));
    }
    app.setFont(QFont(QStringLiteral("Tajawal"), 10));

    vm::Settings settings = vm::Settings::load();
    vm::i18n::set_language(settings.language);

    const QStringList args = app.arguments();
    bool start_hidden = args.contains(QStringLiteral("--minimized")) && !settings.first_run;

    vm::ui::MainWindow win(settings, start_hidden);
    return app.exec();
}
