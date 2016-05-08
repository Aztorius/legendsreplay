#-------------------------------------------------
#
# Project created by QtCreator 2016-03-19T19:59:33
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = LegendsReplay
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    recorder.cpp \
    replay.cpp

HEADERS  += mainwindow.h \
    recorder.h \
    replay.h \
    qhttp/include/qhttpserver.hpp

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/qhttp/lib/ -lqhttp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/qhttp/lib/ -lqhttpd
else:unix: LIBS += -L$$PWD/qhttp/lib/ -lqhttp

INCLUDEPATH += $$PWD/qhttp/include
DEPENDPATH += $$PWD/qhttp/include

RC_FILE = legendsreplay.rc
RESOURCES = legendsreplay.qrc
