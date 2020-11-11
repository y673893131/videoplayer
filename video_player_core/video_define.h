#ifndef VIDEO_DEFINE_H
#define VIDEO_DEFINE_H

#include <string>
#include <functional>
#include <list>
#include <thread>
#include <vector>
#include "video_player_core.h"
#include "Log/Log.h"
#if defined(WIN32)
#include <windows.h>
#endif

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/time.h"
    #include "libavutil/pixfmt.h"
    #include "libavutil/opt.h"
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"

    #include <SDL.h>
    #include <SDL_audio.h>
    #include <SDL_types.h>
    #include <SDL_name.h>
    #include <SDL_main.h>
    #include <SDL_config.h>
}
#include <map>
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
//Log(Log_Info, "new count:%d %p %s %s(%d)",++newCount, *x, #y, __FUNCTION__, __LINE__);
#define SAFE_RELEASE_PTR(x) DELETE_TYPE(*x, "delete count:") if(*x){delete *x;*x=nullptr;}
//Log(Log_Info, "delete count:%d %p",++deleteCount, *x); if(*x){delete *x;*x=nullptr;}
#define CALC_PTS(x, y) ((x - y) / 1000000.0)
#define CALC_DELAY(x, y) ((x) > (y) ? (y) : (x))

#include <mutex>
struct _graud_lock_ : public std::mutex{
    _graud_lock_()
    {
    }

    virtual ~_graud_lock_()
    {
    }
};

#define LOCK(x) std::unique_lock<std::mutex> lck(x);
struct _ffmpeg_packets_
{
#define FFMPEG_PK_SIZE_DEFAULT 1000
#define FFMPEG_SEEK "ffmpeg_seek"
    _ffmpeg_packets_()
        :m_maxSize(FFMPEG_PK_SIZE_DEFAULT)
    {
    }

    _ffmpeg_packets_(int maxSize)
        :m_maxSize(maxSize)
    {
    }

    virtual ~_ffmpeg_packets_()
    {
        clear();
    }

    int size()
    {
        LOCK(mutex)
        return m_packets.size();
    }

    bool empty()
    {
        return m_packets.empty();
    }

    bool empty(AVPacket& pk)
    {
        LOCK(mutex)
        bool bEmpty = m_packets.empty();
        if(!bEmpty)
        {
            pk = m_packets.front();
            m_packets.pop_front();
        }

        return bEmpty;
    }

    void clear()
    {
        LOCK(mutex)
        for(auto& it : m_packets)
        {
            av_packet_unref(&it);
        }

        m_packets.clear();
    }

    bool isMax()
    {
        return m_maxSize <= m_packets.size();
    }

    bool push_back(const char* msg)
    {
        LOCK(mutex)
        if(isMax()) return false;
        AVPacket pk;
        av_new_packet(&pk, strlen(msg) + 1);
        strcpy((char*)pk.data, msg);
        m_packets.push_back(pk);
        return true;
    }

    bool push_back(const AVPacket& pk)
    {
        LOCK(mutex)
        if(isMax()) return false;
        m_packets.push_back(pk);
        return true;
    }

    int m_maxSize;
    _graud_lock_ mutex;
    std::list<AVPacket> m_packets;
};

struct _ffmpeg_out_frame_
{
    _ffmpeg_out_frame_()
        :frame(nullptr)
        ,buffer(nullptr)
        ,swsCov(nullptr)
        ,srcCodec(nullptr)
        ,size(0)
        ,w(0)
        ,h(0)
        ,srcWidth(0)
        ,srcHeight(0)
    {

    }

    virtual ~_ffmpeg_out_frame_()
    {
        uninit();
    }

    void setSize(int w, int h)
    {
        init(w, h);
    }
#ifdef FRAME_RGB
    #define SELECT_PIX_FMT AV_PIX_FMT_RGB24
#else
    #define SELECT_PIX_FMT AV_PIX_FMT_YUV420P
#endif
    void setsrcCodec(AVCodecContext* codec)
    {
        srcCodec = codec;
        init();
    }

    void init(int width = 0, int height = 0)
    {
        LOCK(mutex)
        Log(Log_Info, "buffer0:%p, w=%d, h=%d", buffer, w, h);
        uninit();
        if(width) w = width;
        if(height) h = height;

        if(!srcCodec)
            return;

        initFrame();
//        srcWidth = srcCodec->width;
//        srcHeight = srcCodec->height;
//        frame = av_frame_alloc();
//        swsCov = sws_getContext(srcCodec->width, srcCodec->height, srcCodec->pix_fmt
//                       , w, h, SELECT_PIX_FMT
//                       , SWS_BICUBIC, nullptr, nullptr, nullptr);
//        size = avpicture_get_size(SELECT_PIX_FMT, w, h);
//        buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
//        avpicture_fill((AVPicture*)frame, buffer,SELECT_PIX_FMT, w, h);
    }

    void uninit()
    {
        if(frame)
        {
            av_frame_free(&frame);
            frame = nullptr;
        }

        if(buffer)
        {
            av_free(buffer);
            buffer = nullptr;
        }

        if(swsCov)
        {
            sws_freeContext(swsCov);
            swsCov = nullptr;
        }
    }

    void initFrame()
    {
        uninit();
        if(!w) w = srcCodec->width;
        if(!h) h = srcCodec->height;
        // 16pix align
        w = (w >> 4) << 4;
        h = (h >> 4) << 4;
        srcWidth = srcCodec->width;
        srcHeight = srcCodec->height;
        frame = av_frame_alloc();
        swsCov = sws_getContext(srcCodec->width, srcCodec->height, srcCodec->pix_fmt
                       , w, h, SELECT_PIX_FMT
                       , SWS_BICUBIC, nullptr, nullptr, nullptr);
        size = avpicture_get_size(SELECT_PIX_FMT, w, h);
        buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
        avpicture_fill((AVPicture*)frame, buffer,SELECT_PIX_FMT, w, h);
        Log(Log_Info, "srcWidth=%d, srcHeight=%d size=%d, w=%d, h=%d", srcWidth, srcHeight, size, w, h);
    }

    void scale(AVFrame* src, void* cb)
    {
        if(!cb) return;
        if(srcWidth != srcCodec->width || srcHeight != srcCodec->height)
            initFrame();
        int nRet = sws_scale(swsCov, (const uint8_t *const *)src->data
                  ,src->linesize, 0, srcCodec->height, frame->data
                  ,frame->linesize);
//        Log(Log_Info, "ret:%d, height: %d", nRet, h);
        ((video_interface*)cb)->displayCall(buffer, w, h);
    }
    AVFrame* frame;
    uint8_t* buffer;
    SwsContext* swsCov;
    AVCodecContext* srcCodec;
    _graud_lock_ mutex;
    int size;
    int w;
    int h;
    int srcWidth;
    int srcHeight;
};

struct _ffmpeg_video_info_
{
    _ffmpeg_video_info_()
        :_clock(0)
        ,nStreamIndex(-1)
    {

    }
    AVCodecContext* pCodecContext;
    AVCodec* pCodec;
    _ffmpeg_packets_ pks;
    double _clock;
    std::function<void ()> callback;
    int nStreamIndex;
};

struct _audio_sample_
{
    //2声道 16bit 44100 PCM
    _audio_sample_()
        :channels(2)
        ,fmt(AV_SAMPLE_FMT_S16)
        ,rate(44100)
    {
        layout = av_get_default_channel_layout(channels);
        layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    _audio_sample_(const AVCodecContext* ctx)
        :channels(ctx->channels)
        ,fmt(ctx->sample_fmt)
        ,rate(ctx->sample_rate)
        ,layout(ctx->channel_layout)
    {
        if(layout <= 0)
            layout = av_get_default_channel_layout(layout);
    }

    int linesize()
    {
        return rate * channels;
    }
    int channels;
    AVSampleFormat fmt;
    int rate;
    int64_t layout;
};

typedef void (*cb)(void*, unsigned char*, int);

struct _sdl_op_
{
    _sdl_op_()
        :nAudioId(-1)
        ,nBuffIndex(0)
        ,nBuffSize(0)
        ,fVolPercent(1)
        ,nVol(SDL_MIX_MAXVOLUME)
        ,swrCtx(nullptr)
    {
    }

    bool initResample(AVCodecContext* ctx)
    {
        if(swrCtx)
        {
            swr_free(&swrCtx);
            swrCtx = nullptr;
        }

        // resample: in->out
        in = _audio_sample_(ctx);
        swrCtx = swr_alloc_set_opts(nullptr,
                                         out.layout, out.fmt, out.rate,
                                         in.layout, in.fmt, in.rate,
                                         0, nullptr);
        int nRet = 0;
        if((nRet = swr_init(swrCtx)) < 0)
        {
            char err[128] = {};
            av_strerror(nRet, err, 128);
            Log(Log_Warning, "open resample failed, %s", err);
            swr_free(&swrCtx);
            swrCtx = nullptr;
            return false;
        }

        return true;
    }

    bool init(cb callback, void* userdata)
    {
        int nRet = 0;
        if(SDL_WasInit(nRet))
        {
            return true;
        }

        if((nRet = initSDL()))
        {
            Log(Log_Warning, "sdl init failed, %d", nRet);
            return false;
        }

        target.channels = out.channels;
        target.freq = out.rate;
        target.format = AUDIO_S16SYS;
        target.silence = 0;
        target.samples = 1024;
        target.callback = callback;
        target.userdata = userdata;

        if(openSDL() < 0)
            return false;

        return true;
    }

    virtual ~_sdl_op_()
    {
//        /*if(!nInit)*/ SDL_Quit();
        pauseSDL();
        closeSDL();
        if(swrCtx)
        {
            swr_free(&swrCtx);
            swrCtx = nullptr;
        }

        SDL_Quit();
    }

    int initSDL()
    {
        /*static*/ int nInit = SDL_Init(SDL_INIT_AUDIO);
        return nInit;
    }

    int openSDL()
    {
        auto num = SDL_GetNumAudioDevices(0);
        for(int i = 0; i <= num; ++i)
        {
            auto name = SDL_GetAudioDeviceName(i, 0);
            nAudioId = SDL_OpenAudioDevice(name, false, &target, &spec, 0);
            Log(Log_Info, "audio device id=%d name%d=%s!", nAudioId, i, name);
            if(nAudioId > 0) return nAudioId;
        }

        Log(Log_Warning, "sdl open failed, audio devices count:%d, %s", num, SDL_GetError());
        return -1;
    }

    void closeSDL()
    {
        Log(Log_Info, "sdl close:%d, thread:%d", nAudioId, std::this_thread::get_id());
        if(nAudioId > 0)
        {
//            SDL_LockAudioDevice(nAudioId);
//            Log(Log_Info, "sdl close:1");
//            SDL_PauseAudioDevice(nAudioId, 1);
//            Log(Log_Info, "sdl close:2");
//            SDL_UnlockAudioDevice(nAudioId);
//            Log(Log_Info, "sdl close:3");
            Log(Log_Info, "sdl close:%d, thread:%d, 0x%p", nAudioId, std::this_thread::get_id(), this);
            SDL_LockAudioDevice(nAudioId);
            SDL_CloseAudioDevice(nAudioId);
            SDL_UnlockAudioDevice(nAudioId);
            Log(Log_Info, "sdl close:successed, thread: %d", std::this_thread::get_id());
            nAudioId = -1;
        }
    }

    void startSDL()
    {
        if(nAudioId > 0)
        {
            SDL_LockAudioDevice(nAudioId);
            SDL_PauseAudioDevice(nAudioId, 0);
            SDL_UnlockAudioDevice(nAudioId);
        }
    }

    void pauseSDL()
    {
        if(nAudioId > 0)
        {
            SDL_LockAudioDevice(nAudioId);
            SDL_PauseAudioDevice(nAudioId, 1);
            SDL_UnlockAudioDevice(nAudioId);
        }
    }
#ifdef unix
    typedef signed char         int8_t;
    typedef signed short        int16_t;
    typedef signed int          int32_t;
    typedef unsigned char       uint8_t;
    typedef unsigned short      uint16_t;
    typedef unsigned int        uint32_t;
    typedef unsigned long       DWORD;
    typedef int                 BOOL;
    typedef unsigned char       BYTE;
    typedef unsigned short      WORD;
    typedef float               FLOAT;
    typedef FLOAT               *PFLOAT;
    typedef int                 INT;
    typedef unsigned int        UINT;
    typedef unsigned int        *PUINT;

    typedef unsigned long ULONG_PTR, *PULONG_PTR;
    typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
    #define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
    #define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
    #define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
    #define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
    #define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
    #define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#endif
    int nAudioId;
    _audio_sample_ in;
    _audio_sample_ out;
    SDL_AudioSpec spec;
    SDL_AudioSpec target;
    DECLARE_ALIGNED(16,uint8_t,buff) [192000 * 4];
    unsigned int nBuffIndex;
    unsigned int nBuffSize;
    float fVolPercent;
    int nVol;
    SwrContext *swrCtx;
};

struct _ffmpeg_audio_info_
{
    _ffmpeg_audio_info_()
        :pFrame(nullptr)
        ,pFrameReSample(nullptr)
        ,_clock(0)
        ,nStreamIndex(-1)
    {
        INIT_NEW(&sdl, _sdl_op_);
    }

    virtual ~_ffmpeg_audio_info_()
    {
        if(pFrame)
        {
            av_frame_free(&pFrame);
            pFrame = nullptr;
        }
        if(pFrameReSample)
        {
            av_frame_free(&pFrameReSample);
            pFrameReSample = nullptr;
        }

//        SAFE_RELEASE_PTR(&sdl);
    }
    AVFrame* pFrame;
    AVFrame* pFrameReSample;
    _ffmpeg_packets_ pks;
    double _clock;
    int nStreamIndex;
    static _sdl_op_* sdl;
};

enum flag_bit{
    flag_bit_seek = 0,
    flag_bit_vseek_finish,
    flag_bit_aseek_finish,
    flag_bit_pause,
    flag_bit_Stop,
    flag_bit_tvideo_finish,
    flag_bit_taudio_finish,
    flag_bit_mute,
    flag_bit_read_finish,
    flag_bit_need_pause
};

inline bool isSetBit(unsigned int flag, int bit)
{
    return ((flag >> bit) & 1) == 1;
}

inline void setBit(unsigned int& flag, int bit, bool value = true)
{
    if(!value)
        flag &= ~(1 << bit);
    else
        flag |= (1 << bit);
}

inline bool checkSeek(AVPacket &pk, AVStream *stream)
{
    if(strcmp((char*)pk.data, FFMPEG_SEEK) == 0)
    {
        avcodec_flush_buffers(stream->codec);
        av_packet_unref(&pk);
        return false;
    }

    return true;
}

struct _video_info_
{
    _video_info_()
        :state(0)
        ,_start_time(0)
        ,_seek_time(0)
        ,_pause_time(0)
        ,_seek_pos(0)
        ,_cb(nullptr)
        ,_flag(0)
        ,_format_ctx(nullptr)
        ,yuv(nullptr)
        ,video(nullptr)
        ,audio(nullptr)
        ,_vRead(0)
        ,_aRead(0)
    {

    }

    _video_info_(const _video_info_& src)
        :state(src.state)
        ,src(src.src)
        ,_start_time(src._seek_time)
        ,_seek_time(src._seek_pos)
        ,_pause_time(src._pause_time)
        ,_seek_pos(src._seek_time)
        ,_cb(src._cb)
        ,_flag(src._flag)
        ,_format_ctx(src._format_ctx)
        ,yuv(nullptr)
        ,video(nullptr)
        ,audio(nullptr)
        ,_vRead(src._vRead)
        ,_aRead(src._aRead)
    {
        init();
        audio->sdl->fVolPercent = src.audio->sdl->fVolPercent;
    }
    virtual ~_video_info_()
    {
        if(_format_ctx)
        {
            avformat_close_input(&_format_ctx);
            avformat_free_context(_format_ctx);
            _format_ctx = nullptr;
        }

        SAFE_RELEASE_PTR(&yuv);
        SAFE_RELEASE_PTR(&video);
        SAFE_RELEASE_PTR(&audio);
    }

    void init()
    {
        INIT_NEW(&yuv, _ffmpeg_out_frame_);
        INIT_NEW(&video, _ffmpeg_video_info_);
        INIT_NEW(&audio, _ffmpeg_audio_info_);
    }
    int state;
    std::string src;
    double _start_time, _seek_time, _pause_time;
    int64_t _seek_pos;
    void* _cb;
    unsigned int _flag;
    AVFormatContext* _format_ctx;
    _ffmpeg_out_frame_* yuv;
    _ffmpeg_video_info_* video;
    _ffmpeg_audio_info_* audio;

    int _vRead, _aRead;
};

#endif // VIDEO_DEFINE_H
