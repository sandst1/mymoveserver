#-------------------------------------------------
#
# Project created by QtCreator 2011-08-20T17:19:45
#
#-------------------------------------------------

QT       += core dbus network

QT       -= gui

TARGET = mymoveserver
CONFIG   += console
CONFIG   -= app_bundle

CONFIG += link_prl

CONFIG += mobility
MOBILITY += sensors

TEMPLATE = app

INCLUDEPATH += /usr/X11R6/include
#LIBS += -lXi -lX11 /home/topi/QtSDK/Madde/sysroots/harmattan-meego-arm-sysroot-1122-slim/usr/lib/libfann.a
LIBS += -lXi -lX11 /usr/lib/libfann.a

#DEFINES += ANN_TRAINING

SOURCES += main.cpp \
    mymoveserver.cpp \
    eventhandler.cpp \
    feedbackplayer.cpp

unix:!symbian:!maemo5 {
    target.path = /opt/mymoves/bin
    INSTALLS += target
}

nnetwork.path = /opt/mymoves
nnetwork.files = data/mymoves_simple.net
INSTALLS += nnetwork

initscript.path = /etc/init/apps
initscript.files = mymoveserver.conf
INSTALLS += initscript

mymoveserversh.path = /opt/mymoves/bin
mymoveserversh.files = mymoveserver.sh
INSTALLS += mymoveserversh

scripts.path = /opt/mymoves/mymovescripts
scripts.files = scripts/*
INSTALLS += scripts

feedback.path = /usr/share/themes/base/meegotouch/mymoveserver/feedbacks/priority2_gesture
feedback.files = feedbacks/*
INSTALLS += feedback

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog

HEADERS += \
    mymoveserver.h \
    eventhandler.h \
    feedbackplayer.h

RESOURCES += \
    resources.qrc


