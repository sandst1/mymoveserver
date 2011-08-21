#-------------------------------------------------
#
# Project created by QtCreator 2011-08-20T17:19:45
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

TARGET = mymoveserver
CONFIG   += console
CONFIG   -= app_bundle

CONFIG += mobility
MOBILITY += sensors

TEMPLATE = app

INCLUDEPATH += /usr/X11R6/include
LIBS += -L/usr/X11R6/lib -lXtst -lX11

SOURCES += main.cpp \
    mymoveserver.cpp \
    eventhandler.cpp

unix:!symbian:!maemo5 {
    target.path = /opt/mymoveserver/bin
    INSTALLS += target
}

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog

HEADERS += \
    mymoveserver.h \
    eventhandler.h
