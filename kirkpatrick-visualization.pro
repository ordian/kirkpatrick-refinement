TEMPLATE = app
TARGET = kirkpatrick

CONFIG += QtGui
QT += opengl

OBJECTS_DIR = bin

QMAKE_CXXFLAGS = -std=c++11 -Wall -pedantic -Werror

macx {
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_LFLAGS += -lc++
}

CONFIG += precompile_header
PRECOMPILED_HEADER = src/stdafx.h

DEPENDPATH += src \
              visualization/headers \
              visualization/headers/common \
              visualization/headers/io \
              visualization/headers/visualization \

INCLUDEPATH += src \
               visualization/headers \

HEADERS += src/stdafx.h \
           src/viewer.h \
           src/determinant.h \
           src/graph.h \
           src/homogeneous.h \
           src/triangle.h \
           src/kirkpatrick_refinement.h \

SOURCES += src/main.cpp \
           src/kirkpatrick_refinement.cpp \
           src/triangle.cpp \
           src/io.cpp \

LIBS += -Lvisualization -lvisualization
