#-------------------------------------------------
#
# Project created by QtCreator 2016-07-13T07:52:30
#
#-------------------------------------------------

QT       += core gui location xml quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QPokentroller
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    MapController.cpp \
    XcodeController.cpp

HEADERS  += MainWindow.h \
    MapController.h \
    XcodeController.h

FORMS    += MainWindow.ui

DISTFILES += \
    change-xcode-location.applescript \
    Map.qml

applescript.files = $$DISTFILES
applescript.path = Contents/Resources
QMAKE_BUNDLE_DATA += applescript

RESOURCES += \
    qmlSources.qrc
