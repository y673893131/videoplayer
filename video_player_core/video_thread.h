#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H
#include <memory>
#include <vector>
#include "video_define.h"
#ifdef unix
#include <unistd.h>
#define msleep(x) usleep(x * 1000)
#elif(WIN32)
#define msleep(x) Sleep(x)
#endif

class video_thread
{
public:
    virtual ~video_thread();
    static void start(const _video_info_&);
    static video_thread* index(int);
    void play();
    void setPause();
    bool getPause();
    void continuePlay();
    void setStop();
    bool getStop();
    void seekPos(int64_t);
    void setVol(int);
    void setMute(bool bMute);
    int state();
private:
    video_thread(std::shared_ptr<_video_info_>);
    video_thread(const video_thread&);
    video_thread(const _video_info_&);
    void startPlay();
    void video_decode();
    bool audio_decode_prepare();
    void decode_loop();
    void seek();
    bool push_frame();
private:
    int m_index;
    _video_info_* m_info;
    static std::vector<video_thread*> m_threads;
};

#endif // VIDEO_THREAD_H
