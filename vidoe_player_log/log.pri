CONFIG -= qt

TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

#win32{
#CONFIG += dll
#}

DEFINES += VIDEO_PLAYER_LOG_LIBRARY

SOURCES += $$PWD/Log/Log.cpp
HEADERS += $$PWD/Log/Log.h
