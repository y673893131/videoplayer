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
    render/qglvideowidget.h \
    render/qlabelvideowidget.h \
    render/qd3d11widget.h \
    render/qdirect3d11widget.h \
    render/dx11/cameraclass.h \
    render/dx11/d3dclass.h \
    render/dx11/inputclass.h \
    render/dx11/textureclass.h \
    render/dx11/textureshaderclass.h

SOURCES += \
    $$PWD/control/videocontrol.cpp \
    $$PWD/framelesswidget/nativeevent_win.cpp \
    $$PWD/framelesswidget/util.cpp \
    $$PWD/render/dx11/bitmapclass.cpp \
    $$PWD/render/dx11/graphicsclass.cpp \
    $$PWD/render/qrenderfactory.cpp \
    render/qglvideowidget.cpp \
    render/qlabelvideowidget.cpp \
    render/qd3d11widget.cpp \
    render/qdirect3d11widget.cpp \
    render/dx11/cameraclass.cpp \
    render/dx11/d3dclass.cpp \
    render/dx11/inputclass.cpp \
    render/dx11/textureclass.cpp \
    render/dx11/textureshaderclass.cpp

SOURCES += \
    $$PWD/config.cpp \
    $$PWD/croommanager.cpp \
    $$PWD/main.cpp \  
    $$PWD/qdouyuwidget.cpp \
    $$PWD/qfilelistdelegate.cpp \
    $$PWD/qfilelistview.cpp \    
    $$PWD/qnetworkqueue.cpp \ 
    $$PWD/qplayfilelistmodel.cpp \
    $$PWD/qtoolwidgets.cpp \
    $$PWD/widget.cpp \
    $$PWD/framelesswidget/framelesswidget.cpp \
    $$PWD/framelesswidget/qdragborder.cpp \
    $$PWD/progress/qprogressslider.cpp \
    $$PWD/qdatamodel.cpp \
    $$PWD/qinputurlwidget.cpp \
    $$PWD/platform/platform/qlivelist.cpp \
    $$PWD/platform/platform/qlivelistmodel.cpp \
    $$PWD/platform/platform/qliveplatformmanager.cpp \
    $$PWD/platform/model/qegamemodel.cpp \
    $$PWD/platform/model/qdymodel.cpp \
    $$PWD/platform/model/qhuyamodel.cpp \
    $$PWD/platform/model/qlivedatamodel.cpp \
    $$PWD/platform/room/qpagewidget.cpp \
    $$PWD/platform/room/qroomlistdelegate.cpp \
    $$PWD/platform/room/qroomlistview.cpp \
    $$PWD/platform/room/qroomlistviewmodel.cpp

HEADERS += \
    $$PWD/config.h \
    $$PWD/croommanager.h \
    $$PWD/datadef.h \
    $$PWD/qdouyuwidget.h \
    $$PWD/qfilelistdelegate.h \
    $$PWD/qfilelistview.h \
    $$PWD/qnetworkqueue.h \   
    $$PWD/qplayfilelistmodel.h \  
    $$PWD/qtoolwidgets.h \
    $$PWD/videoframe.h \
    $$PWD/widget.h \
    $$PWD/framelesswidget/framelesswidget.h \
    $$PWD/framelesswidget/qdragborder.h \
    $$PWD/progress/qprogressslider.h \
    $$PWD/qdatamodel.h \
    $$PWD/qinputurlwidget.h \
    $$PWD/platform/platform/qlivelist.h \
    $$PWD/platform/platform/qlivelistmodel.h \
    $$PWD/platform/platform/qliveplatformmanager.h \
    $$PWD/platform/model/qegamemodel.h \
    $$PWD/platform/model/qdymodel.h \
    $$PWD/platform/model/qhuyamodel.h \
    $$PWD/platform/model/qlivedatamodel.h \
    $$PWD/platform/room/qpagewidget.h \
    $$PWD/platform/room/qroomlistdelegate.h \
    $$PWD/platform/room/qroomlistview.h \
    $$PWD/platform/room/qroomlistviewmodel.h

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
