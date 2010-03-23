TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT += opengl xml
CONFIG += debug

LIBS += -lGeographic

QMAKE_CXXFLAGS += -fopenmp -g
QMAKE_LFLAGS += -fopenmp -g

# Input
HEADERS += gpxgui.h

SOURCES += main.cpp \
           gpxgui.cpp

RESOURCES += gpxgui.qrc
