TEMPLATE = app
TARGET = gpxgui
DEPENDPATH += .
INCLUDEPATH += . ../qtgpxlib
QT += opengl xml
CONFIG += debug

LIBS += -lGeographic -lqtgpxlib

QMAKE_LFLAGS += -L../qtgpxlib

# Input
HEADERS += gpxgui.h

SOURCES += main.cpp \
           gpxgui.cpp

RESOURCES += gpxgui.qrc

unix:QMAKE_POST_LINK=strip $$TARGET
