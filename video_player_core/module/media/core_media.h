#ifndef CORE_MEDIA_H
#define CORE_MEDIA_H

#include "../decoder/core_decoder_video.h"
#include "../decoder/core_decoder_audio.h"
#include "../decoder/core_decoder_subtitle.h"
#include "../preview/core_preview.h"
#include "../thread/core_thread_video.h"
#include "../thread/core_thread_audio.h"
#include "../thread/core_thread_subtitle.h"
#include "../filter/core_filter.h"
#include "../save/core_save.h"
#include "video_player_core.h"

class core_media
{
public:
    core_media();
    core_media(const core_media& src);
    virtual ~core_media();

    void init();
    void uninit();

    // out call
    void setCallback(video_interface* cb);
    void setSrc(const std::string&);
    void setSize(int,int);
    void play();
    void setPause();
    bool isPause();
    void continuePlay();
    void stop();
    bool isStop();
    bool seek(int64_t);
    bool jump(int64_t);
    void preview(int64_t);
    void setVol(int);
    void mute(bool bMute);
    void setAudioChannel(int type);
    void setChannel(int, int);
    void setDecode(int type);
    void setSpeed(int type);
    int state();
    video_player_core::enum_state state1();
    int setCapture(bool);


    // decode
    bool open(int index);
    int videoIndex();
    int audioIndex();
    void decodeloop();
private:
    static int interruptCallback(void* ctx);
    void setFlag(int bit, bool value = true);
    bool testFlag(int bit);
    void setState(int state);
    void seek();
    int getSeekFlag(core_decoder* decoder, int64_t);
    void pushSeekPkt();
    void channelChange();
    bool push_frame(bool& bSeek);
    bool checkSeekPkt(core_decoder* decoder, AVPacket* pk);
    int64_t start_time();
public:
    video_interface* _cb;
private:
    int _state;
    std::string _src;
    int64_t _start_time, _pause_time;
    int64_t _seek_pos;
    int64_t _seek_step;
    unsigned int _flag;
    AVFormatContext* _format_ctx;
    core_decoder_video* _video;
    core_decoder_audio* _audio;
    core_decoder_subtitle* _subtitle;
    std::vector<_stream_channel_info_*> _channels[channel_max];
    unsigned int _channel, _channelSel;
    int _decodeType;
    int _speed;
    bool _capture;

    core_preview* m_preview;
    core_thread_audio* _audio_thread;
    core_thread_video* _video_thread;
    core_thread_subtitle* _subtitle_thread;
    core_save* _save;
    friend class core_thread;
    friend class core_thread_audio;
    friend class core_thread_demux;
    friend class core_thread_video;
    friend class core_thread_subtitle;
    friend class core_filter;
    friend class core_save_base;
};

#endif // CORE_MEDIA_H
