#-------------------------------------------------
#
# Project created by QtCreator 2016-06-19T11:08:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VariantWidgetsTest
TEMPLATE = app


SOURCES += main.cpp \
    metawrapper.cpp

HEADERS  += \
	metawrapper.h

include(../loadLib.pri)

TRANSLATIONS += trans_VariantWidgetsTest_cn.ts
