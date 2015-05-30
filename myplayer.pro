#-------------------------------------------------
#
# Project created by QtCreator 2015-05-11T21:29:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myplayer
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    playlistwidget.cpp \
    SliderPoint.cpp

HEADERS  += widget.h \
    playlistwidget.h \
    SliderPoint.h

FORMS    += widget.ui

DISTFILES +=

RESOURCES += \
    myplayer.qrc
