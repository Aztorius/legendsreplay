#-------------------------------------------------
#
# Project created by QtCreator 2016-03-19T19:59:33
#
#-------------------------------------------------

QT       += core gui network widgets

PRJDIR = ..

TARGET = LegendsReplay
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    recorder.cpp \
    replay.cpp \
    chunk.cpp \
    keyframe.cpp \
    gameinfoswidget.cpp \
    server.cpp \
    advancedrecorderdialog.cpp

HEADERS  += mainwindow.h \
    recorder.h \
    replay.h \
    chunk.h \
    keyframe.h \
    gameinfoswidget.h \
    server.h \
    advancedrecorderdialog.h

FORMS    += mainwindow.ui \
    gameinfoswidget.ui \
    advancedrecorderdialog.ui

unix: LIBS += -L$$OUT_PWD/../qhttp/xbin/ -lqhttp
win32: LIBS += -L$$OUT_PWD/../qhttp/xbin/ -lqhttp

INCLUDEPATH += $$PRJDIR/qhttp/src
DEPENDPATH += $$PRJDIR/qhttp/src

CONFIG += c++11

RC_ICONS = logo-win32.ico
RESOURCES = legendsreplay.qrc
