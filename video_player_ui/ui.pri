CONFIG += c++11

SOURCES += \
    framelesswidget/framelesswidget.cpp \
    main.cpp \
    qglvideowidget.cpp \
    widget.cpp \
    qlabelvideowidget.cpp

HEADERS += \
    framelesswidget/framelesswidget.h \
    qglvideowidget.h \
    widget.h \
    qlabelvideowidget.h

INCLUDEPATH += $$PWD/../vidoe_player_log
DEPENDPATH += $$PWD/../vidoe_player_log
LIBS += -L$$PWD/../bin -lvidoe_player_log

win32{
    INCLUDEPATH += $$PWD/../video_player_core
    LIBS += -L$$PWD/../bin -lvideo_player_core
    DESTDIR = $$PWD/../bin
}

HEADERS += \
    $$PWD/glvdieodefine.h \
    $$PWD/videoframe.h
