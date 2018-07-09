#-------------------------------------------------
#
# Project created by QtCreator 2014-12-11T16:35:02
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = tpool_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH = ../src

DEFINES += ENABLE_TPOOL_DBG

SOURCES += main.cpp \
    testobject.cpp \
    workerobject.cpp \
    ../src/nrthreadpool.cpp

HEADERS += \
    testobject.h \
    workerobject.h \
    ../src/nrthreadpool.h
