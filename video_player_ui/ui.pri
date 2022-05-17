CONFIG += c++11

win32{
QMAKE_LFLAGS_DEBUG += /MAP
QMAKE_LFLAGS_DEBUG += /Zi
QMAKE_LFLAGS_DEBUG += /debug /opt:ref
QMAKE_LFLAGS_RELEASE += /MAP
QMAKE_CFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /debug /opt:ref
}

win32 {
#    admintrator setting
#    Tips: admin need administrator startup QtCreator
#    QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"
#    QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,\"5.01\"

    LIBS += -luser32 -lopengl32 -ldwmapi -lgdi32 -lole32

DEFINES += RENDER_DX11
contains(DEFINES, RENDER_DX11) {
    INCLUDEPATH += $$(DXSDK_DIR)\Include
    LIBS += -L$$(DXSDK_DIR)\lib\x86 -ld3d11 -ld3dx11 -ld3dcompiler

HEADERS += \
    $$PWD/render/qdirect3d11widget.h \
    $$PWD/render/dx11/freqclass.h \
    $$PWD/render/dx11/bitmapclass.h \
    $$PWD/render/dx11/graphicsclass.h \
    $$PWD/render/dx11/cameraclass.h \
    $$PWD/render/dx11/d3dclass.h \
    $$PWD/render/dx11/inputclass.h \
    $$PWD/render/dx11/textureclass.h \
    $$PWD/render/dx11/textureshaderclass.h
SOURCES += \
    $$PWD/render/qdirect3d11widget.cpp \
    $$PWD/render/dx11/freqclass.cpp \
    $$PWD/render/dx11/bitmapclass.cpp \
    $$PWD/render/dx11/graphicsclass.cpp \
    $$PWD/render/dx11/cameraclass.cpp \
    $$PWD/render/dx11/d3dclass.cpp \
    $$PWD/render/dx11/inputclass.cpp \
    $$PWD/render/dx11/textureclass.cpp \
    $$PWD/render/dx11/textureshaderclass.cpp
}

DEFINES += THUMBNAIL
contains(DEFINES, THUMBNAIL) {
QT += winextras
    HEADERS += \
        $$PWD/ui/thumb/qwinthumbnail_p.h \
        $$PWD/ui/thumb/qwinthumbnail.h \
        $$PWD/ui/thumb/qwintaskbarmenu.h
    SOURCES += \
        $$PWD/ui/thumb/qwinthumbnail_p.cpp \
        $$PWD/ui/thumb/qwinthumbnail.cpp \
        $$PWD/ui/thumb/qwintaskbarmenu.cpp
}
}

win32 {
HEADERS += \
    $$PWD/framelesswidget/nativeevent_win.h
SOURCES += \
    $$PWD/framelesswidget/nativeevent_win.cpp
}
linux {
HEADERS += \
    $$PWD/framelesswidget/nativeevent_uinx.h
SOURCES += \
    $$PWD/framelesswidget/nativeevent_uinx.cpp
}

# ui
HEADERS += \
    $$PWD/config/configDef.h \
    $$PWD/control/videocontrol.h \
    $$PWD/dump/mini_dump.hpp \
    $$PWD/filter/qinputfilter.h \\
    $$PWD/framelesswidget/nativeevent_p.h \
    $$PWD/framelesswidget/qframelesswidget_p.h \
    $$PWD/framelesswidget/util.h \
    $$PWD/render/native/qnativewidget.h \
    $$PWD/render/native/qnativewidget_p.h \
    $$PWD/render/qrenderfactory.h \
    $$PWD/render/qglvideowidget.h \
    $$PWD/render/qlabelvideowidget.h \
    $$PWD/render/qrenderprivate.h \
    $$PWD/render/videoframe.h \
    $$PWD/ui/qtoolwidgets.h \
    $$PWD/ui/tool/base/qsubtitlelabel.h \
    $$PWD/ui/tool/subtitle/engine/apidef.h \
    $$PWD/ui/tool/subtitle/engine/libass/qlibassinterface.h \
    $$PWD/ui/tool/subtitle/engine/qassengineer.h \
    $$PWD/ui/tool/subtitle/engine/qplayerenginner.h \
    $$PWD/ui/tool/subtitle/engine/qsubtitleengineer.h \
    $$PWD/ui/tool/subtitle/engine/qsubtitlengine.h \
    $$PWD/ui/tool/subtitle/engine/libass/libass.h \
    $$PWD/ui/widget.h \
    $$PWD/ui/qvolumewidget.h \
    $$PWD/ui/tool/base/qtoolbase.h \
    $$PWD/ui/tool/fileview/qfileview.h \
    $$PWD/ui/tool/fileview/playlist/qplayfilelistmodel.h \
    $$PWD/ui/tool/fileview/playlist/qfilelistdelegate.h \
    $$PWD/ui/tool/fileview/playlist/qfilelistview.h \
    $$PWD/ui/tool/menu/qfilelistmenu.h \
    $$PWD/ui/tool/menu/qplaymenu.h \
    $$PWD/ui/tool/menu/qplaymenubase.h \
    $$PWD/ui/tool/output/qoutputwidget.h \
    $$PWD/ui/tool/play_control/qplaycontrol.h \
    $$PWD/ui/tool/progress/qprogressslider.h \
    $$PWD/ui/tool/progress/qplayprogress.h \
    $$PWD/ui/tool/subtitle/qplaysubtitle.h \
    $$PWD/ui/tool/title/qplaytitle.h \
    $$PWD/ui/tool/volume/qcolumeslider.h

SOURCES += \
    $$PWD/control/videocontrol.cpp \
    $$PWD/filter/qinputfilter.cpp \
    $$PWD/framelesswidget/nativeevent_p.cpp \
    $$PWD/framelesswidget/qframelesswidget_p.cpp \
    $$PWD/framelesswidget/util.cpp \
    $$PWD/render/native/qnativewidget.cpp \
    $$PWD/render/native/qnativewidget_p.cpp \
    $$PWD/render/qrenderfactory.cpp \
    $$PWD/render/qrenderprivate.cpp \
    $$PWD/render/videoframe.cpp \
    $$PWD/render/qglvideowidget.cpp \
    $$PWD/render/qlabelvideowidget.cpp \
    $$PWD/ui/qvolumewidget.cpp \
    $$PWD/ui/qtoolwidgets.cpp \
    $$PWD/ui/tool/base/qsubtitlelabel.cpp \
    $$PWD/ui/tool/subtitle/engine/libass/qlibassinterface.cpp \
    $$PWD/ui/tool/subtitle/engine/qassengineer.cpp \
    $$PWD/ui/tool/subtitle/engine/qplayerenginner.cpp \
    $$PWD/ui/tool/subtitle/engine/qsubtitleengineer.cpp \
    $$PWD/ui/tool/subtitle/engine/qsubtitlengine.cpp \
    $$PWD/ui/tool/subtitle/engine/libass/libass.cpp \
    $$PWD/ui/widget.cpp \
    $$PWD/ui/tool/base/qtoolbase.cpp \
    $$PWD/ui/tool/fileview/qfileview.cpp \
    $$PWD/ui/tool/fileview/playlist/qfilelistdelegate.cpp \
    $$PWD/ui/tool/fileview/playlist/qfilelistview.cpp \
    $$PWD/ui/tool/fileview/playlist/qplayfilelistmodel.cpp \
    $$PWD/ui/tool/menu/qfilelistmenu.cpp \
    $$PWD/ui/tool/menu/qplaymenu.cpp \
    $$PWD/ui/tool/menu/qplaymenubase.cpp \
    $$PWD/ui/tool/output/qoutputwidget.cpp \
    $$PWD/ui/tool/play_control/qplaycontrol.cpp \
    $$PWD/ui/tool/progress/qprogressslider.cpp \
    $$PWD/ui/tool/progress/qplayprogress.cpp \
    $$PWD/ui/tool/subtitle/qplaysubtitle.cpp \
    $$PWD/ui/tool/title/qplaytitle.cpp \
    $$PWD/ui/tool/volume/qcolumeslider.cpp

SOURCES += \
    $$PWD/config/config.cpp \
    $$PWD/config/qdatamodel.cpp \
    $$PWD/main.cpp \
    $$PWD/framelesswidget/framelesswidget.cpp
HEADERS += \
    $$PWD/config/config.h \
    $$PWD/config/qdatamodel.h \
    $$PWD/framelesswidget/framelesswidget.h

#pop
SOURCES += \
    $$PWD/ui/pop/qpopwidget.cpp \
    $$PWD/ui/pop/qopenlocalfilewidget.cpp \
    $$PWD/ui/pop/qinputurlwidget.cpp \
    $$PWD/ui/pop/qmediainfowidget.cpp \
    $$PWD/ui/pop/qbitratedisplay.cpp \
    $$PWD/ui/pop/qcreateitem.cpp \
    $$PWD/ui/pop/qpopwidgetprivate.cpp
HEADERS += \
    $$PWD/ui/pop/qpopwidget.h \
    $$PWD/ui/pop/qopenlocalfilewidget.h \
    $$PWD/ui/pop/qinputurlwidget.h \
    $$PWD/ui/pop/qmediainfowidget.h \
    $$PWD/ui/pop/qbitratedisplay.h \
    $$PWD/ui/pop/qcreateitem.h \
    $$PWD/ui/pop/qpopwidgetprivate.h

#business expansion
#DEFINES += GAME_PLATFORM_LIVE
contains(DEFINES, GAME_PLATFORM_LIVE) {
QT += script
HEADERS += \
    $$PWD/ui/tool/live_platform/qliveplatform.h \
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
    $$PWD/platform/room/qroomlistviewmodel.h
SOURCES += \
    $$PWD/ui/tool/live_platform/qliveplatform.cpp \
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
    $$PWD/platform/room/qroomlistviewmodel.cpp
}
else
{
    message("no GAME_PLATFORM_LIVE business expansion")
}

#aas
DEFINES += AAS_RENDER
contains(DEFINES, AAS_RENDER) {
INCLUDEPATH += \
    $$PWD/ui/tool/subtitle/engine/libass/c/include

#ASS_C_LOAD is loadlibrary
DEFINES += ASS_C_LOAD
!contains(DEFINES, ASS_C_LOAD) {
LIBS += -L$$PWD/ui/tool/subtitle/engine/libass/c/lib/x86 -lass
}
}

INCLUDEPATH += \
    $$PWD \
    $$PWD/../vidoe_player_log \
    $$PWD/../video_player_core \
    $$PWD/../video_pimpl

LIBS += -L$$PWD/../bin -lvidoe_player_log -lvideo_player_core

CONFIG += debug_and_release


#win32-g++ {
#    LIBS += -lopengl32 -lglu32
#}
#win32-msvc*{
#    LIBS += opengl32.lib glu32.lib glmf32.lib
#}

DESTDIR = $$PWD/../bin

RESOURCES += \
    $$PWD/res.qrc

TRANSLATIONS = $$PWD/tr/zh_cn/vPlayTranslate.ts

RC_FILE += \
    $$PWD/res.rc

TARGET = vPlay
