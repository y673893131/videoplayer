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
    void getSeekImg(int64_t);
    void setVol(int);
    void setMute(bool bMute);
    void setChannel(int, int);
    void setDecode(video_player_core::enum_decode_type);
    int state();
    video_player_core::enum_state state1();
private:
    video_thread(std::shared_ptr<core_media>);
    video_thread(const video_thread&);
    video_thread(const core_media&);
    void startPlay();
//    void video_decode();
//    bool audio_decode_prepare();
//    void decode_loop();
//    double get_video_pts(AVFrame* frame);
//    void seek();
//    bool push_frame(bool& bSeek);
//    bool checkSeekPkt(AVPacket* pk);
//    void cleanPkt(int channel = -1);
//    void cleanStreamPkt(int channel);
    // subtitle
//    bool checkSubTitle(AVPacket*);
    // channel
//    void channelChange();
private:
    int m_index;
    core_media* m_media;
    static std::vector<video_thread*> m_threads;
};

#endif // VIDEO_THREAD_H
