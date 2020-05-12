#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H
#include <memory>
#include "video_define.h"
class video_thread
{
public:
    video_thread(std::shared_ptr<_video_info_>);
    virtual ~video_thread();

    static void start(_video_info_*);
private:
    void startPlay();
    void video_decode();
    bool audio_decode_prepare();
    int audio_decode();
    void decode_loop();
    static void sdl_audio_call(void *data, Uint8 *stream, int len);
    void audio_call(Uint8*, int);
    void seek();
    bool push_frame();
    bool checkSeek(AVPacket& pk, AVStream* stream);
private:
    std::shared_ptr<_video_info_> m_info;
};

#endif // VIDEO_THREAD_H
