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

CONFIG += link_pkgconfig
PKGCONFIG += fann

TEMPLATE = app

INCLUDEPATH += /usr/X11R6/include
LIBS += -lXi -lX11

#DEFINES += ANN_TRAINING

SOURCES += main.cpp \
    mymoveserver.cpp \
    eventhandler.cpp

unix:!symbian:!maemo5 {
    target.path = /opt/mymoveserver/bin
    INSTALLS += target
}

nnetwork.path = /opt/mymoveserver
nnetwork.files = data/mymoves_nn1.net data/mymoves_nn2.net data/mymoves_nn3.net
INSTALLS += nnetwork

initscript.path = /etc/init/apps
initscript.files = mymoveserver.conf
INSTALLS += initscript

mymoveserversh.path = /opt/mymoveserver/bin
mymoveserversh.files = mymoveserver.sh
INSTALLS += mymoveserversh

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

RESOURCES += \
    resources.qrc
