#-------------------------------------------------
#
# Project created by QtCreator 2016-03-19T19:59:33
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = OpenReplay
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    recorder.cpp \
    replay.cpp \
    server.cpp

HEADERS  += mainwindow.h \
    recorder.h \
    replay.h \
    server.h

FORMS    += mainwindow.ui
