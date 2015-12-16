#-------------------------------------------------
#
# Project created by QtCreator 2015-11-16T19:01:48
#
#-------------------------------------------------

QT       += core gui \
            widgets \
            printsupport \
            network \
            serialport

TARGET = Groundstation_prel
TEMPLATE = app

SOURCES += main.cpp \
    groundstation.cpp \
    compass.cpp \
    debrismap.cpp \
    qcustomplot.cpp \
    console.cpp \
    connection.cpp \
    payload.cpp

HEADERS  += groundstation.h \
    compass.h \
    debrismap.h \
    qcustomplot.h \
    console.h \
    connection.h \
    payload.h

FORMS    += groundstation.ui
