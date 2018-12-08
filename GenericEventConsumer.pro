# -------------------------------------------------
# Project created by QtCreator 2010-06-21T11:02:26
# -------------------------------------------------
#QT -= gui
TARGET = GenericEventConsumer
VERSION = 1.2
DESTDIR = ./bin/
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    genericconsumer.cpp
HEADERS += genericconsumer.h

INCLUDEPATH += ../../include/
INCLUDEPATH += ../../icore/

FORMS += \
    form.ui

DEFINES += __x86__
unix {
    DEFINES += __OS_VERSION__=2 \
        __linux__
    OMNIBASE = /usr/local
    INCLUDEPATH += /usr/include/omniORB4 \
        /usr/include/COS
    LIBS += -L../../lib \
        -licore
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}
win32 {
    INCLUDEPATH += $$(OMNIORB_BASE)/include/
    DEFINES += __WIN32__ \
        WIN32 \
        _WIN32_WINNT=0x0400 \
        __NT__ \
        __OSVERSION__=4
    LIBS += -L$$(OMNIORB_BASE)/lib/x86_win32\ \
        -lomniDynamic4_rt \
        -lomniORB4_rt \
        -lomnithread_rt
    LIBS += ../../lib/icore.lib
    MOC_DIR = _moc
    OBJECTS_DIR = _obj
}
