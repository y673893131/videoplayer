CONFIG += c++11

SOURCES += \
    $$PWD/qfilelistdelegate.cpp \
    $$PWD/qfilelistview.cpp \
    $$PWD/qplayfilelistmodel.cpp \
    $$PWD/qtoolwidgets.cpp \
    framelesswidget/framelesswidget.cpp \
    main.cpp \
    qglvideowidget.cpp \
    widget.cpp \
    qlabelvideowidget.cpp

HEADERS += \
    $$PWD/qfilelistdelegate.h \
    $$PWD/qfilelistview.h \
    $$PWD/qplayfilelistmodel.h \
    $$PWD/qtoolwidgets.h \
    framelesswidget/framelesswidget.h \
    qglvideowidget.h \
    widget.h \
    qlabelvideowidget.h \
    $$PWD/glvdieodefine.h \
    $$PWD/videoframe.h

INCLUDEPATH += $$PWD/../vidoe_player_log
DEPENDPATH += $$PWD/../vidoe_player_log
LIBS += -L$$PWD/../bin -lvidoe_player_log

win32{
    INCLUDEPATH += $$PWD/../video_player_core
    LIBS += -L$$PWD/../bin -lvideo_player_core
    DESTDIR = $$PWD/../bin
}

RESOURCES += \
    $$PWD/res.qrc
