CONFIG += c++11

INCLUDEPATH += $$PWD/../vidoe_player_log
LIBS += -L$$PWD/../bin -lvidoe_player_log

SOURCES += \
    $$PWD/audio_thread.cpp \
    $$PWD/video_player_core.cpp \
    $$PWD/video_thread.cpp

HEADERS += \
    $$PWD/audio_thread.h \
    $$PWD/video_define.h \
    $$PWD/video_player_core.h \
    $$PWD/video_thread.h

win32{
    contains(QT_ARCH, i386){
        message("build-32bit")
        INCLUDEPATH += $$PWD/lib/win32/ffmpeg/include \
                       $$PWD/lib/win32/SDL2/include
    }else{
        message("build-64bit")
        INCLUDEPATH += $$PWD/lib/win64/ffmpeg/include \
                       $$PWD/lib/win64/SDL2/include
    }

    LIBS += -L$$PWD/lib/win32/ffmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
    LIBS += -L$$PWD/lib/win32/SDL2/lib -lSDL2
}

unix{
    contains(QT_ARCH, i386) {
        message("32-bit, 请自行编译32位库!")
    } else {
        message("linux 64-bit")
        message($$OUT_PWD)
        message($$PWD)
        INCLUDEPATH += $$PWD/lib/linux/ffmpeg/include \
                       $$PWD/lib/linux/SDL2/include/SDL2 \
                       $$PWD/src

        LIBS += -L$$PWD/lib/linux/ffmpeg/lib  -lavformat  -lavcodec -lavdevice -lavfilter -lavutil -lswresample -lswscale
        LIBS += -L$$PWD/lib/linux/SDL2/lib -lSDL2

        LIBS += -lpthread -ldl
    }
}
