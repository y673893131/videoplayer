CONFIG += c++11

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/qfilelistdelegate.cpp \
    $$PWD/qfilelistview.cpp \
    $$PWD/qglvideowidget.cpp \
    $$PWD/qlabelvideowidget.cpp \
    $$PWD/qplayfilelistmodel.cpp \
    $$PWD/qtoolwidgets.cpp \
    $$PWD/widget.cpp \
    $$PWD/framelesswidget/framelesswidget.cpp \
    $$PWD/framelesswidget/qdragborder.cpp \
    $$PWD/qprogressslider.cpp \
    $$PWD/qdatamodel.cpp

HEADERS += \
    $$PWD/qfilelistdelegate.h \
    $$PWD/qfilelistview.h \
    $$PWD/qglvideowidget.h \
    $$PWD/qlabelvideowidget.h \
    $$PWD/qplayfilelistmodel.h \
    $$PWD/qtoolwidgets.h \
    $$PWD/videoframe.h \
    $$PWD/widget.h \
    $$PWD/framelesswidget/framelesswidget.h \
    $$PWD/framelesswidget/qdragborder.h \
    $$PWD/qprogressslider.h \
    $$PWD/qdatamodel.h

INCLUDEPATH += \
    $$PWD \
    $$PWD/../vidoe_player_log \
    $$PWD/../video_player_core
LIBS += -L$$PWD/../bin -lvidoe_player_log -lvideo_player_core
DESTDIR = $$PWD/../bin

RESOURCES += \
    $$PWD/res.qrc

TRANSLATIONS = vPlayTranslate.ts
