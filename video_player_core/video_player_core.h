#ifndef VIDEO_PLAYER_CORE_H
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

//#include "video_player_core_global.h"
#include <functional>
struct _video_info_;
class VIDEO_PLAYER_CORE_EXPORT video_interface
{
public:
    virtual void totalTime(const _int64 t) = 0;
    virtual void displayCall(void* data, int width, int height) = 0;
    virtual void endCall() = 0;
};

class VIDEO_PLAYER_CORE_EXPORT video_player_core
{
public:
    enum enum_operator
    {
        opr_stop,
        opr_start,
        opr_pause,
        opr_continue
    };

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
    int _pause();
    int _continue();
    int _stop();
    int _seek(_int64);
    int _setsize(int w, int h);
    bool _cov(void *indata, void* outdata, int w, int h, int outsize);
    enum_state _getState();
private:
    _video_info_* m_info;
};

#endif // VIDEO_PLAYER_CORE_H
