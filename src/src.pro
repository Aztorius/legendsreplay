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
    keyframe.cpp

HEADERS  += mainwindow.h \
    recorder.h \
    replay.h \
    chunk.h \
    keyframe.h

FORMS    += mainwindow.ui

unix: LIBS += -L$$OUT_PWD/../qhttp/xbin/ -lqhttp
win32: LIBS += -L$$OUT_PWD/../qhttp/xbin/ -lqhttp

INCLUDEPATH += $$PRJDIR/qhttp/src
DEPENDPATH += $$PRJDIR/qhttp/src

CONFIG += c++11

RC_FILE = legendsreplay.rc
RESOURCES = legendsreplay.qrc
