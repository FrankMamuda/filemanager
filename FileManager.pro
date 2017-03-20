#-------------------------------------------------
#
# Project created by QtCreator 2017-01-19T11:55:05
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    pixmapcache.cpp \
    pathutils.cpp \
    properties.cpp \
    tableview.cpp \
    entry.cpp \
    containermodel.cpp \
    listview.cpp \
    sideview.cpp \
    textutils.cpp \
    listviewdelegate.cpp \
    tableviewdelegate.cpp \
    bookmark.cpp \
    bookmarkmodel.cpp \
    notificationpanel.cpp \
    history.cpp

HEADERS  += mainwindow.h \
    pixmapcache.h \
    pathutils.h \
    properties.h \
    tableview.h \
    main.h \
    entry.h \
    containermodel.h \
    listview.h \
    sideview.h \
    textutils.h \
    variable.h \
    listviewdelegate.h \
    tableviewdelegate.h \
    bookmark.h \
    bookmarkmodel.h \
    containerstyle.h \
    notificationpanel.h \
    history.h \
    textlabel.h
    common.h

FORMS    += mainwindow.ui \
    properties.ui \
    notificationpanel.ui

RESOURCES += \
    resources.qrc

win32:RC_FILE = icon.rc

