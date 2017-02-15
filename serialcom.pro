#-------------------------------------------------
#
# Project created by QtCreator 2016-08-16T13:29:16
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = serialcom
TEMPLATE = app


SOURCES += \
    main.cpp \
    posix_qextserialport.cpp \
    qextserialbase.cpp \
    win_qextserialport.cpp \
    mainwindow.cpp \
    win_mysql_query.cpp \
    com_send_thread.cpp

HEADERS  += \
    posix_qextserialport.h \
    qextserialbase.h \
    win_qextserialport.h \
    mainwindow.h \
    win_mysql_query.h \
    global.h \
    com_send_thread.h

FORMS    += \
    mainwindow.ui

RESOURCES += \
    res.qrc

RC_ICONS = images/icon.ico
