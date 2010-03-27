TEMPLATE   = app
CONFIG    += console
SOURCES   += gpxfile.cpp gpxtracksegment.cpp gpxpoint.cpp testutm.cpp
LIBS += -lGeographic
QMAKE_CXXFLAGS += -g
QMAKE_LFLAGS += -g
QT += xml
QT -= gui

