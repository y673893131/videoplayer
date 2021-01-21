CONFIG += c++11

win32
{
    INCLUDEPATH += F:/DX11/Include
    LIBS += -LF:/DX11/lib/x86 -ld3d11 -ld3dx11 -ld3dcompiler
}

# ui
HEADERS += \
    $$PWD/control/videocontrol.h \
    $$PWD/framelesswidget/nativeevent_win.h \
    $$PWD/framelesswidget/util.h \
    $$PWD/render/dx11/bitmapclass.h \
    $$PWD/render/dx11/graphicsclass.h \
    $$PWD/render/qrenderfactory.h \
    $$PWD/render/qglvideowidget.h \
    $$PWD/render/qlabelvideowidget.h \
    $$PWD/render/qd3d11widget.h \
    $$PWD/render/qdirect3d11widget.h \
    $$PWD/render/dx11/cameraclass.h \
    $$PWD/render/dx11/d3dclass.h \
    $$PWD/render/dx11/inputclass.h \
    $$PWD/render/dx11/textureclass.h \
    $$PWD/render/dx11/textureshaderclass.h

SOURCES += \
    $$PWD/control/videocontrol.cpp \
    $$PWD/framelesswidget/nativeevent_win.cpp \
    $$PWD/framelesswidget/util.cpp \
    $$PWD/render/dx11/bitmapclass.cpp \
    $$PWD/render/dx11/graphicsclass.cpp \
    $$PWD/render/qrenderfactory.cpp \
    $$PWD/render/videoframe.h \
    $$PWD/render/qglvideowidget.cpp \
    $$PWD/render/qlabelvideowidget.cpp \
    $$PWD/render/qd3d11widget.cpp \
    $$PWD/render/qdirect3d11widget.cpp \
    $$PWD/render/dx11/cameraclass.cpp \
    $$PWD/render/dx11/d3dclass.cpp \
    $$PWD/render/dx11/inputclass.cpp \
    $$PWD/render/dx11/textureclass.cpp \
    $$PWD/render/dx11/textureshaderclass.cpp

SOURCES += \
    $$PWD/config/config.cpp \
    $$PWD/config/qdatamodel.cpp \
    $$PWD/main.cpp \  
    $$PWD/ui/qtoolwidgets.cpp \
    $$PWD/ui/widget.cpp \
    $$PWD/ui/qinputurlwidget.cpp \
    $$PWD/framelesswidget/framelesswidget.cpp \
    $$PWD/framelesswidget/qdragborder.cpp \
    $$PWD/progress/qprogressslider.cpp \
    $$PWD/platform/platform/qdouyuwidget.cpp \
    $$PWD/platform/platform/qlivelist.cpp \
    $$PWD/platform/platform/qlivelistmodel.cpp \
    $$PWD/platform/platform/qliveplatformmanager.cpp \
    $$PWD/platform/model/qegamemodel.cpp \
    $$PWD/platform/model/qdymodel.cpp \
    $$PWD/platform/model/qhuyamodel.cpp \
    $$PWD/platform/model/qlivedatamodel.cpp \
    $$PWD/platform/model/qnetworkqueue.cpp \
    $$PWD/platform/room/croommanager.cpp \
    $$PWD/platform/room/qpagewidget.cpp \
    $$PWD/platform/room/qroomlistdelegate.cpp \
    $$PWD/platform/room/qroomlistview.cpp \
    $$PWD/platform/room/qroomlistviewmodel.cpp \
    $$PWD/playlist/qfilelistdelegate.cpp \
    $$PWD/playlist/qfilelistview.cpp \
    $$PWD/playlist/qplayfilelistmodel.cpp

HEADERS += \
    $$PWD/config/config.h \
    $$PWD/config/qdatamodel.h \
    $$PWD/ui/qtoolwidgets.h \
    $$PWD/ui/widget.h \
    $$PWD/ui/qinputurlwidget.h \
    $$PWD/framelesswidget/framelesswidget.h \
    $$PWD/framelesswidget/qdragborder.h \
    $$PWD/progress/qprogressslider.h \
    $$PWD/platform/platform/qdouyuwidget.h \
    $$PWD/platform/platform/qlivelist.h \
    $$PWD/platform/platform/qlivelistmodel.h \
    $$PWD/platform/platform/qliveplatformmanager.h \
    $$PWD/platform/model/datadef.h \
    $$PWD/platform/model/qegamemodel.h \
    $$PWD/platform/model/qdymodel.h \
    $$PWD/platform/model/qhuyamodel.h \
    $$PWD/platform/model/qlivedatamodel.h \
    $$PWD/platform/model/qnetworkqueue.h \
    $$PWD/platform/room/croommanager.h \
    $$PWD/platform/room/qpagewidget.h \
    $$PWD/platform/room/qroomlistdelegate.h \
    $$PWD/platform/room/qroomlistview.h \
    $$PWD/platform/room/qroomlistviewmodel.h \
    $$PWD/playlist/qplayfilelistmodel.h \
    $$PWD/playlist/qfilelistdelegate.h \
    $$PWD/playlist/qfilelistview.h

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

TRANSLATIONS = $$PWD/tr/zh_cn/vPlayTranslate.ts

RC_FILE += \
    $$PWD/res.rc

TARGET = vPlay
