TEMPLATE = app
TARGET = gpxgui
DEPENDPATH += .
INCLUDEPATH += . ../qtgpxlib
QT += opengl xml
CONFIG += debug

LIBS += -lGeographic -lqtgpxlib

QMAKE_LFLAGS += -L../qtgpxlib

# Input
HEADERS += gpxgui.h gpxtreewidget.h unitconversion.h

SOURCES += main.cpp \
           gpxgui.cpp gpxtreewidget.cpp unitconversion.cpp

RESOURCES += gpxgui.qrc

unix:QMAKE_POST_LINK=strip $$TARGET
