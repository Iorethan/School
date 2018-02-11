#-------------------------------------------------
#
# Project created by QtCreator 2017-10-11T13:33:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = xvales03
TEMPLATE = app

QMAKE_MAKEFILE = xvales03_makefile

SOURCES += main.cpp\
        window.cpp \
    implication.cpp \
    resources/qcustomplot.cpp

HEADERS  += window.h \
    implication.h \
    resources/qcustomplot.h

FORMS    += window.ui

DISTFILES += \
    resources/changelog.txt \
    resources/GPL.txt
