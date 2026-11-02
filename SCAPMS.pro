# qmake project file (alternative to CMake) - open this in Qt Creator.
QT += core gui widgets
CONFIG += c++17
TEMPLATE = app
TARGET = SCAPMS

SOURCES += src/main.cpp \
    $$files(src/gui/*.cpp) \
    $$files(src/model/*.cpp) \
    $$files(src/controller/*.cpp) \
    $$files(src/utils/*.cpp)

HEADERS += \
    $$files(src/gui/*.h) \
    $$files(src/model/*.h) \
    $$files(src/controller/*.h) \
    $$files(src/utils/*.h)
