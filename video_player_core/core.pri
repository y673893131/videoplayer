CONFIG += c++11

INCLUDEPATH += $$PWD/../vidoe_player_log \
    $$PWD/module/third

LIBS += -L$$PWD/../bin -lvidoe_player_log

SOURCES += \
    $$PWD/module/convert/core_frame_convert.cpp \
    $$PWD/module/decoder/core_decoder.cpp \
    $$PWD/module/decoder/core_decoder_audio.cpp \
    $$PWD/module/decoder/core_decoder_hardware.cpp \
    $$PWD/module/decoder/core_decoder_subtitle.cpp \
    $$PWD/module/decoder/core_decoder_video.cpp \
    $$PWD/module/lock/core_graud_lock.cpp \
    $$PWD/module/media/core_media.cpp \
    $$PWD/module/packet/core_packets.cpp \
    $$PWD/module/preview/core_preview.cpp \
    $$PWD/module/sample/core_audio_sample.cpp \
    $$PWD/module/sdl/core_sdl_op.cpp \
    $$PWD/module/thread/core_thread.cpp \
    $$PWD/module/thread/core_thread_audio.cpp \
    $$PWD/module/thread/core_thread_video.cpp \
    $$PWD/module/util/core_util.cpp \
    $$PWD/video_player_core.cpp \
    $$PWD/video_thread.cpp

HEADERS += \
    $$PWD/module/common.h \
    $$PWD/module/convert/core_frame_convert.h \
    $$PWD/module/decoder/core_decoder.h \
    $$PWD/module/decoder/core_decoder_audio.h \
    $$PWD/module/decoder/core_decoder_hardware.h \
    $$PWD/module/decoder/core_decoder_subtitle.h \
    $$PWD/module/decoder/core_decoder_video.h \
    $$PWD/module/lock/core_graud_lock.h \
    $$PWD/module/media/core_media.h \
    $$PWD/module/packet/core_packets.h \
    $$PWD/module/preview/core_preview.h \
    $$PWD/module/sample/core_audio_sample.h \
    $$PWD/module/sdl/core_sdl_op.h \
    $$PWD/module/thread/core_thread.h \
    $$PWD/module/thread/core_thread_audio.h \
    $$PWD/module/thread/core_thread_video.h \
    $$PWD/module/util/core_util.h \
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
