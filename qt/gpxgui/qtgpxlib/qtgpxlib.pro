TEMPLATE = lib
CONFIG += staticlib

SOURCES = gpxfile.cpp gpxpoint.cpp gpxtracksegment.cpp
HEADERS = gpxelement.h gpxfile.h gpxpoint.h gpxtracksegment.h track.h

LIBS += -lGeographic

QT += xml

QMAKE_CXXFLAGS += -g
QMAKE_LFLAGS += -g 
