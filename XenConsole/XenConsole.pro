#-------------------------------------------------
#
# Project created by QtCreator 2014-01-31T21:51:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XenConsole
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

DEFINES += QTONLY

LIBS += -lcurl -lxml2 -lxenserver -lvncclient

SOURCES += main.cpp\
        mainwindow.cpp \
    authenticateserverdialog.cpp \
    krdc_VncView/vncview.cpp \
    krdc_VncView/vncclientthread.cpp \
    krdc_VncView/remoteview.cpp \
    xsconsolevnctunnel.cpp \
    vmwindow.cpp \
    xsloadhostthread.cpp \
    xenserverapi.cpp

HEADERS  += mainwindow.h \
    xenserverapi.h \
    authenticateserverdialog.h \
    krdc_VncView/vncview.h \
    krdc_VncView/vncclientthread.h \
    krdc_VncView/remoteview.h \
    xsconsolevnctunnel.h \
    vmwindow.h \
    xsloadhostthread.h

FORMS    += mainwindow.ui \
    authenticateserverdialog.ui \
    vmwindow.ui
