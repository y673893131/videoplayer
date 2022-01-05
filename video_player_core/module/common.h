#ifndef COMMON_H
#define COMMON_H
#include <sstream>
#include <thread>
#include <functional>
#include "Log/Log.h"

//#define _DESKTOP_
#define USE_AVFILTER
#define AUDIO_FILTER
#define VIDEO_FILTER

extern "C"
{
    #include "libavdevice/avdevice.h"
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/time.h"
    #include "libavutil/pixfmt.h"
    #include "libavutil/opt.h"
    #include "libavutil/imgutils.h"
    //need intel media sdk
    #include "libavutil/hwcontext_qsv.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
#ifdef USE_AVFILTER
    #include "libavfilter/avfilter.h"
    #include"libavfilter/buffersrc.h"
    #include"libavfilter/buffersink.h"
#endif

    #include <SDL.h>
    #include <SDL_audio.h>
    #include <SDL_types.h>
    #include <SDL_name.h>
    #include <SDL_main.h>
    #include <SDL_config.h>
}

//#define CHECK_MEM_LEAK
#ifdef CHECK_MEM_LEAK
static int newCount = 0;
static int deleteCount = 0;
static std::map<void*, std::string> leakMap;

#define NEW_TYPE(x, y) leakMap[x] = y;
#define DELETE_TYPE(x, y) {auto it = leakMap.find(x); if(it != leakMap.end()) leakMap.erase(it);}
#define OUT_PUT_LEAK() Log(Log_Info, "mem leak size:%d", leakMap.size()); for(auto it : leakMap){ Log(Log_Info, "mem leak:%p %s", it.first, it.second.c_str()); }
#else
#define NEW_TYPE(x, y)
#define DELETE_TYPE(x, y)
#define OUT_PUT_LEAK()
#endif
#define INIT_NEW(x, y) if(!*x) *x = new y(); NEW_TYPE(*x, ":"#y)
#define SAFE_RELEASE_PTR(x) DELETE_TYPE(*x, "delete count:") if(*x){delete *x;*x=nullptr;}
#define CALC_PTS(x, y) ((x - y) / 1000000.0)
#define CALC_DELAY(x, y) ((x) > (y) ? (y) : (x))


#ifdef unix
#include <unistd.h>
#define msleep(x) usleep(x * 1000)
#elif(WIN32)
#include <Windows.h>
#define msleep(x) Sleep(static_cast<unsigned long>(x))
#endif

#endif // COMMON_H
