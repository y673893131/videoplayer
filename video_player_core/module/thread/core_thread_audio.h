#ifndef CORE_THREAD_AUDIO_H
#define CORE_THREAD_AUDIO_H

#include "../common.h"
#include "core_thread.h"
class core_thread_audio : public core_thread
{
public:
    static core_thread_audio *instance();
    bool start(core_media* media) override;
    static void audio_callback(void *data, Uint8 *stream, int len);
private:
    core_thread_audio();
    ~core_thread_audio() override;
    void audio_call(Uint8 *stream, unsigned int len);
    unsigned int audio_decode();
private:
    static core_thread_audio* s_instance;
    unsigned int m_nBuffIndex;
    unsigned int m_nBuffSize;
};

#endif // CORE_THREAD_AUDIO_H
