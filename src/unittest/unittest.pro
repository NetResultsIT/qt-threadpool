QT += testlib network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_test_threadallocationpolicy.cpp \
    ../nrthreadpool.cpp

HEADERS += \
    ../nrthreadpool.h
