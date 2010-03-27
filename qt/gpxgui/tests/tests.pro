TEMPLATE   = app
CONFIG    += console
SOURCES   += tests.cpp
LIBS += -lGeographic -lqtgpxlib

INCLUDEPATH += ../ ../qtgpxlib

QMAKE_CXXFLAGS += -g
QMAKE_LFLAGS += -g -L../qtgpxlib

QT += xml

unix:QMAKE_POST_LINK=./tests
