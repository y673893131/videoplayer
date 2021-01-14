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

//#define FRAME_RGB

struct _video_info_;
class VIDEO_PLAYER_CORE_EXPORT video_interface
{
public:
    virtual void totalTime(const int64_t t) = 0;
    virtual void posChange(const int64_t t) = 0;
    virtual void setVideoSize(int width, int hight) = 0;
    virtual void displayCall(void* data, int width, int height) = 0;
    virtual void displaySubTitleCall(char*) = 0;
    virtual void previewDisplayCall(void* data, int width, int height) = 0;
    virtual void startCall(int) = 0;
    virtual void endCall(int) = 0;
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
    video_player_core();
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
    int _get_seek_img(int, int64_t);
    int _setVol(int, int);
    int _setMute(int, bool bMute);
    int _setsize(int w, int h);
    int _state(int);
    bool _cov(void *indata, void* outdata, int w, int h, int outsize);
    enum_state _getState();
private:
    _video_info_* m_info;
};

#endif // VIDEO_PLAYER_CORE_H
