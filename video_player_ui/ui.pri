CONFIG += c++11

SOURCES += \
    $$PWD/config.cpp \
    $$PWD/croommanager.cpp \
    $$PWD/main.cpp \
    $$PWD/platform/qegamemodel.cpp \
    $$PWD/qdouyuwidget.cpp \
    $$PWD/qfilelistdelegate.cpp \
    $$PWD/qfilelistview.cpp \
    $$PWD/qglvideowidget.cpp \
    $$PWD/qlabelvideowidget.cpp \
    $$PWD/qlivelist.cpp \
    $$PWD/qlivelistmodel.cpp \
    $$PWD/qliveplatformmanager.cpp \
    $$PWD/qnetworkqueue.cpp \
    $$PWD/qpagewidget.cpp \
    $$PWD/qplayfilelistmodel.cpp \
    $$PWD/qroomlistdelegate.cpp \
    $$PWD/qroomlistview.cpp \
    $$PWD/qroomlistviewmodel.cpp \
    $$PWD/qtoolwidgets.cpp \
    $$PWD/widget.cpp \
    $$PWD/framelesswidget/framelesswidget.cpp \
    $$PWD/framelesswidget/qdragborder.cpp \
    $$PWD/qprogressslider.cpp \
    $$PWD/qdatamodel.cpp \
    $$PWD/qinputurlwidget.cpp \
    $$PWD/platform/qdymodel.cpp \
    $$PWD/platform/qhuyamodel.cpp \
    $$PWD/platform/qlivedatamodel.cpp

HEADERS += \
    $$PWD/config.h \
    $$PWD/croommanager.h \
    $$PWD/datadef.h \
    $$PWD/platform/qegamemodel.h \
    $$PWD/qdouyuwidget.h \
    $$PWD/qfilelistdelegate.h \
    $$PWD/qfilelistview.h \
    $$PWD/qglvideowidget.h \
    $$PWD/qlabelvideowidget.h \
    $$PWD/qlivelist.h \
    $$PWD/qlivelistmodel.h \
    $$PWD/qliveplatformmanager.h \
    $$PWD/qnetworkqueue.h \
    $$PWD/qpagewidget.h \
    $$PWD/qplayfilelistmodel.h \
    $$PWD/qroomlistdelegate.h \
    $$PWD/qroomlistview.h \
    $$PWD/qroomlistviewmodel.h \
    $$PWD/qtoolwidgets.h \
    $$PWD/videoframe.h \
    $$PWD/widget.h \
    $$PWD/framelesswidget/framelesswidget.h \
    $$PWD/framelesswidget/qdragborder.h \
    $$PWD/qprogressslider.h \
    $$PWD/qdatamodel.h \
    $$PWD/qinputurlwidget.h \
    $$PWD/platform/qdymodel.h \
    $$PWD/platform/qhuyamodel.h \
    $$PWD/platform/qlivedatamodel.h

INCLUDEPATH += \
    $$PWD \
    $$PWD/../vidoe_player_log \
    $$PWD/../video_player_core
LIBS += -L$$PWD/../bin -lvidoe_player_log -lvideo_player_core

win32-g++ {
    LIBS += -lopengl32 -lglu32
}
win32-msvc*{
    LIBS += opengl32.lib glu32.lib glmf32.lib
}

DESTDIR = $$PWD/../bin

RESOURCES += \
    $$PWD/res.qrc

TRANSLATIONS = vPlayTranslate.ts

RC_FILE += \
    $$PWD/res.rc

TARGET = vPlay
