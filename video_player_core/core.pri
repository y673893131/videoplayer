CONFIG += c++11

win32{
QMAKE_LFLAGS_DEBUG += /MAP
QMAKE_LFLAGS_DEBUG += /Zi
QMAKE_LFLAGS_DEBUG += /debug /opt:ref
QMAKE_LFLAGS_RELEASE += /MAP
QMAKE_CFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /debug /opt:ref
}

INCLUDEPATH += $$PWD/../vidoe_player_log \
    $$PWD/module/third \
    $$PWD/../video_pimpl

LIBS += -L$$PWD/../bin -lvidoe_player_log

SOURCES += \
    $$PWD/module/convert/core_convert.cpp \
    $$PWD/module/convert/core_convert_audio.cpp \
    $$PWD/module/convert/core_convert_p.cpp \
    $$PWD/module/convert/core_convert_video.cpp \
    $$PWD/module/decoder/core_decoder.cpp \
    $$PWD/module/decoder/core_decoder_audio.cpp \
    $$PWD/module/decoder/core_decoder_hardware.cpp \
    $$PWD/module/decoder/core_decoder_subtitle.cpp \
    $$PWD/module/decoder/core_decoder_video.cpp \
    $$PWD/module/dev/core_dev.cpp \
    $$PWD/module/dev/core_dev_sdl.cpp \
    $$PWD/module/filter/core_filter.cpp \
    $$PWD/module/filter/core_filter_audio.cpp \
    $$PWD/module/filter/core_filter_base.cpp \
    $$PWD/module/filter/core_filter_video.cpp \
    $$PWD/module/filter/core_filter_private.cpp \
    $$PWD/module/lock/core_graud_lock.cpp \
    $$PWD/module/media/core_media.cpp \
    $$PWD/module/packet/core_packets.cpp \
    $$PWD/module/preview/core_preview.cpp \
    $$PWD/module/sample/core_audio_sample.cpp \
    $$PWD/module/save/core_save.cpp \
    $$PWD/module/save/core_save_audio.cpp \
    $$PWD/module/save/core_save_base.cpp \
    $$PWD/module/save/core_save_video.cpp \
#    $$PWD/module/sdl/core_sdl_op.cpp \
    $$PWD/module/thread/core_thread.cpp \
    $$PWD/module/thread/core_thread_audio.cpp \
    $$PWD/module/thread/core_thread_demux.cpp \
    $$PWD/module/thread/core_thread_subtitle.cpp \
    $$PWD/module/thread/core_thread_video.cpp \
    $$PWD/module/util/core_util.cpp \
    $$PWD/video_player_core.cpp

HEADERS += \
    $$PWD/module/common.h \
    $$PWD/module/convert/core_convert.h \
    $$PWD/module/convert/core_convert_audio.h \
    $$PWD/module/convert/core_convert_define.h \
    $$PWD/module/convert/core_convert_p.h \
    $$PWD/module/convert/core_convert_video.h \
    $$PWD/module/decoder/core_decoder.h \
    $$PWD/module/decoder/core_decoder_audio.h \
    $$PWD/module/decoder/core_decoder_hardware.h \
    $$PWD/module/decoder/core_decoder_subtitle.h \
    $$PWD/module/decoder/core_decoder_video.h \
    $$PWD/module/dev/core_dev.h \
    $$PWD/module/dev/core_dev_sdl.h \
    $$PWD/module/filter/core_filter.h \
    $$PWD/module/filter/core_filter_audio.h \
    $$PWD/module/filter/core_filter_base.h \
    $$PWD/module/filter/core_filter_video.h \
    $$PWD/module/filter/core_filter_private.h \
    $$PWD/module/lock/core_graud_lock.h \
    $$PWD/module/media/core_media.h \
    $$PWD/module/packet/core_packets.h \
    $$PWD/module/preview/core_preview.h \
    $$PWD/module/sample/core_audio_sample.h \
    $$PWD/module/save/core_save.h \
    $$PWD/module/save/core_save_audio.h \
    $$PWD/module/save/core_save_base.h \
    $$PWD/module/save/core_save_video.h \
#    $$PWD/module/sdl/core_sdl_op.h \
    $$PWD/module/thread/core_thread.h \
    $$PWD/module/thread/core_thread_audio.h \
    $$PWD/module/thread/core_thread_demux.h \
    $$PWD/module/thread/core_thread_subtitle.h \
    $$PWD/module/thread/core_thread_video.h \
    $$PWD/module/util/core_util.h \
    $$PWD/video_player_core.h

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
        message("32-bit!")
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

#        QMAKE_LFLAGS += "-Wl,-rpath,\'$$ORIGING\'"
#        QMAKE_RPATHDIR += :\'\$\$DESTDIR\'
    }
}

DEFINES += AUDIO_WAVE_DISPLAY
contains(DEFINES, AUDIO_WAVE_DISPLAY) {
    INCLUDEPATH += \
        $$PWD/../fftreal

    LIBS += -L$$PWD/../bin -lfftreal
}
else
{
    message("no AUDIO_WAVE_DISPLAY business expansion");
}

win32{
    DEFINES += AUDIO_DEV_SELECT
    contains(DEFINES, AUDIO_DEV_SELECT) {
    SOURCES += \
        $$PWD/module/dev/MMNotificationClient.cpp \
        $$PWD/module/dev/core_dev_dsound.cpp \
        $$PWD/module/dev/core_dev_p.cpp \
        $$PWD/module/dev/core_dev_waveout.cpp \
        $$PWD/module/dev/core_dev_xaudio2.cpp

    HEADERS += \
        $$PWD/module/dev/MMNotificationClient.h \
        $$PWD/module/dev/core_dev_dsound.h \
        $$PWD/module/dev/core_dev_p.h \
        $$PWD/module/dev/core_dev_waveout.h \
        $$PWD/module/dev/core_dev_xaudio2.h \

        LIBS += -lwinmm -lole32
        LIBS += -ldsound -ldxguid -luser32 -lxaudio2 -lavrt
    }
    else
    {
        message("no AUDIO_DEV_SELECT business expansion");
    }
}
