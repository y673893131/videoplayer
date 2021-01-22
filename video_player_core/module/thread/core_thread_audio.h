#ifndef CORE_THREAD_AUDIO_H
#define CORE_THREAD_AUDIO_H

#include "../common.h"
#include "core_thread.h"
class core_thread_audio : public core_thread
{
public:
    virtual ~core_thread_audio();
    static core_thread_audio *instance();
    bool start(core_media* media) override;
    static void sdl_audio_call(void *data, Uint8 *stream, int len);
private:
    core_thread_audio();
    void audio_call(Uint8 *stream, unsigned int len);
    unsigned int audio_decode();
private:
    static core_thread_audio* s_instance;
};

#endif // CORE_THREAD_AUDIO_H
