TEMPLATE   = app
CONFIG    += console
SOURCES   += testutm.cpp
LIBS += -lGeographic
QMAKE_CXXFLAGS += -fopenmp -g
QMAKE_LFLAGS += -fopenmp -g
QT -= gui
QT += xml
