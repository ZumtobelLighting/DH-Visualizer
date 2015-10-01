#-------------------------------------------------
#
# Project created by QtCreator 2015-07-23T16:57:07
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = visualizer
TEMPLATE = app


SOURCES += main.cpp \
           mainwindow.cpp \
           myqgraphicsview.cpp

HEADERS  += mainwindow.h \
            myqgraphicsview.h

FORMS    += mainwindow.ui
