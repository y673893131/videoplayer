#ifndef CORE_MEDIA_H
#define CORE_MEDIA_H

#include "../decoder/core_decoder_video.h"
#include "../decoder/core_decoder_audio.h"
#include "../decoder/core_decoder_subtitle.h"
#include "../preview/core_preview.h"
#include "../thread/core_thread_video.h"
#include "../thread/core_thread_audio.h"
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
    void seek(int64_t);
    void preview(int64_t);
    void setVol(int);
    void mute(bool bMute);
    void setChannel(int, int);
    void setDecode(video_player_core::enum_decode_type type);
    int state();
    video_player_core::enum_state state1();

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
    void cleanPkt(int channel = -1);
    void channelChange();
    void decodeChange();
    bool push_frame(bool& bSeek);
    bool checkSeekPkt(AVPacket* pk);
public:
    video_interface* _cb;
private:
    int _state;
    std::string _src;
    double _start_time, _seek_time, _pause_time;
    int64_t _seek_pos;
    unsigned int _flag;
    AVFormatContext* _format_ctx;
    core_decoder_video* video;
    core_decoder_audio* audio;
    core_decoder_subtitle* subtitle;
    std::vector<_stream_channel_info_*> _channels[channel_max];
    int _vRead, _aRead;
    int _channel, _channelSel;
    video_player_core::enum_decode_type _decodeType;

    core_preview* m_preview;
    core_thread_audio* _audio_thread;
    core_thread_video* _video_thread;
    friend class core_thread;
    friend class core_thread_audio;
    friend class core_thread_video;
};

#endif // CORE_MEDIA_H
