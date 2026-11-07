QT += core gui widgets network
qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += VM_HAS_MULTIMEDIA=1
}
qtHaveModule(positioning) {
    QT += positioning
    DEFINES += VM_HAS_POSITIONING=1
}

CONFIG += c++17
TARGET = V-Mawaqeet
TEMPLATE = app

INCLUDEPATH += src src/ui

HEADERS += \
    src/app_info.h \
    src/paths.h \
    src/config.h \
    src/autostart.h \
    src/hijri.h \
    src/methods.h \
    src/prayer_calc.h \
    src/locations.h \
    src/geo.h \
    src/providers.h \
    src/i18n.h \
    src/theme.h \
    src/workers.h \
    src/state.h \
    src/notifier.h \
    src/sound.h \
    src/ui/icons.h \
    src/ui/art.h \
    src/ui/widgets.h \
    src/ui/locate.h \
    src/ui/notification.h \
    src/ui/about_dialog.h \
    src/ui/home.h \
    src/ui/themes_page.h \
    src/ui/settings_page.h \
    src/ui/onboarding.h \
    src/ui/window.h

SOURCES += \
    src/main.cpp \
    src/paths.cpp \
    src/config.cpp \
    src/autostart.cpp \
    src/hijri.cpp \
    src/methods.cpp \
    src/prayer_calc.cpp \
    src/locations.cpp \
    src/geo.cpp \
    src/providers.cpp \
    src/i18n.cpp \
    src/theme.cpp \
    src/state.cpp \
    src/notifier.cpp \
    src/sound.cpp \
    src/ui/icons.cpp \
    src/ui/art.cpp \
    src/ui/widgets.cpp \
    src/ui/locate.cpp \
    src/ui/notification.cpp \
    src/ui/about_dialog.cpp \
    src/ui/home.cpp \
    src/ui/themes_page.cpp \
    src/ui/settings_page.cpp \
    src/ui/onboarding.cpp \
    src/ui/window.cpp

win32 {
    RC_ICONS = assets/icon.ico
    LIBS += -lshell32 -lole32
}
