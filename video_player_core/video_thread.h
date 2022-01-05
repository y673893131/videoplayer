#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H
#include <memory>
#include <vector>
//#include "video_define.h"
#include "module/media/core_media.h"

class video_thread
{
public:
    virtual ~video_thread();
    static void start(const core_media&);
    static video_thread* index(size_t);
    void setSize(int w, int h);
    void play();
    void setPause();
    bool getPause();
    void continuePlay();
    void setStop();
    bool getStop();
    void seekPos(int64_t);
    bool seekJump(int64_t);
    void getSeekImg(int64_t);
    void setVol(int);
    void setMute(bool bMute);
    void setAudioChannel(audio_channel_type);
    void setChannel(int, int);
    void setDecode(int);
    void setSpeed(int);
    int state();
    video_player_core::enum_state state1();
    int setCapture(bool);
private:
    video_thread(std::shared_ptr<core_media>);
    video_thread(const video_thread&);
    video_thread(const core_media&);
    void startPlay();
private:
    int m_index;
    core_media* m_media;
    static std::vector<video_thread*> m_threads;
};

#endif // VIDEO_THREAD_H
