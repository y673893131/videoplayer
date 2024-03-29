﻿#ifndef VIDEO_PLAYER_CORE_H
#define VIDEO_PLAYER_CORE_H

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define Q_DECL_EXPORT __declspec(dllexport)
#  define Q_DECL_IMPORT __declspec(dllimport)
#else
#  define Q_DECL_EXPORT     __attribute__((visibility("default")))
#  define Q_DECL_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(VIDEO_PLAYER_CORE_LIBRARY)
#  define VIDEO_PLAYER_CORE_EXPORT Q_DECL_EXPORT
#else
#  define VIDEO_PLAYER_CORE_EXPORT Q_DECL_IMPORT
#endif

#ifdef WIN32
typedef long long int64_t;
#endif
#include <functional>
#include <vector>
#include <map>
//#define FRAME_RGB

enum enum_stream_channel
{
    channel_video = 0,
    channel_audio,
    channel_subtitle,

    channel_max
};

struct _stream_channel_info_
{
    _stream_channel_info_()
        : index(-1)
    {

    }

    _stream_channel_info_(const std::string& sTitle, const std::string& sLanguage, int index)
    {
        this->sTitle = sTitle;
        this->sLanguage = sLanguage;
        this->index = index;
    }

    std::string sTitle;
    std::string sLanguage;
    int index;
};

enum audio_channel_type
{
    audio_channel_both,
    audio_channel_left,
    audio_channel_right
};

struct subtitle_info
{
    char sName[128];
    char sFont[128];
    int pt;
    int color[4];//0-frontColor 1-SecondColor 2-ouelineColor 3-shadowColor
    int bold;
    int italic;
    int underLine;
    int strikeOut;
    int scaleX;
    int scaleY;
    int spacing;
    int angle;
    int borderStyle;
    int outlinePix;
    int shadowPix;
    int alignment;
    int marginLeft;
    int marginRight;
    int marginBottom;
    int encoding;
};

struct subtitle_header
{
    subtitle_header()
        : bInit(false)
        , isASS(false)
    {
    }
    bool bInit;
    bool isASS;
    std::string sHeader;
    std::vector<subtitle_info> infos;
};

class core_media;
class VIDEO_PLAYER_CORE_EXPORT video_interface
{
public:
    virtual void supportHWDecoder(const std::map<int, std::string>&) = 0;
    virtual void totalTime(const int64_t t, const char*) = 0;
    virtual void posChange(const int64_t t) = 0;
    virtual void bitRate(const int64_t size) = 0;
    virtual void setVideoSize(int width, int hight) = 0;
    virtual void displayStreamChannelInfo(enum_stream_channel channel, const std::vector<_stream_channel_info_*>&, int defalut) = 0;
    virtual void displayCall(void* data, int width, int height) = 0;
    virtual void subtitleHaderCall(const subtitle_header&) = 0;
    virtual void displaySubTitleCall(char*, unsigned int, int, int64_t start, int64_t end) = 0;
    virtual void displayFreqCall(float*, unsigned int) = 0;
    virtual void previewDisplayCall(void* data, int width, int height) = 0;
    virtual void startCall(int) = 0;
    virtual void endCall(int) = 0;
    virtual void exceptionEndCall(int, const char*) = 0;
};

class VIDEO_PLAYER_CORE_EXPORT video_player_core
{
public:
    enum enum_state
    {
        state_uninit,
        state_running,
        state_paused,
        state_stopped
    };

public:
    explicit video_player_core(const std::string& logDir);
    virtual ~video_player_core();
    int _init();
    int _uninit();
    int _setCallBack(video_interface*);
    int _setSrc(const std::string& src);
    int _play();
    int _pause(int);
    int _continue(int);
    int _stop(int);
    int _seek(int, int64_t);
    bool _seekJump(int, int64_t);
    int _get_seek_img(int, int64_t);
    int _setVol(int, int);
    int _setMute(int, bool bMute);
    int _setAudioChannel(int, audio_channel_type type);
    int _setStreamChannel(int index, int channel, int sel);
    int _setDecodeType(int index, int type);
    int _setSpeedType(int index, int type);
    int _state(int);
    bool _cov(void *indata, void* outdata, int w, int h, int outsize);
    int _setCapture(int,bool);
    enum_state _getState(int);
    void* _frame(int index);
private:
    core_media* m_media;
};

#endif // VIDEO_PLAYER_CORE_H
