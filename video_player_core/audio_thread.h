#ifndef AUDIO_THREAD_H
#define AUDIO_THREAD_H

#include "video_define.h"

class audio_thread
{
public:
    virtual ~audio_thread();

    static audio_thread *instance();
    bool addAudio(AVCodecContext* ctx, AVStream* stream, unsigned int& flag);
    bool addAudio(_video_info_* pInfo);
private:
    audio_thread();
    bool init();
    static void sdl_audio_call(void *data, Uint8 *stream, int len);
    void audio_call(Uint8 *stream, int len);
    int audio_decode();
private:
    static audio_thread* s_instance;
    _video_info_* m_info;
    _ffmpeg_audio_info_* audio;
    AVStream* m_stream;
    unsigned int* m_flag;
};

#endif // AUDIO_THREAD_H
