#ifndef VIDEO_DEFINE_H
#define VIDEO_DEFINE_H

#include <string>
#include <functional>
#include <list>
#include <thread>
#include "video_player_core.h"
#include "Log/Log.h"
#if defined(WIN32)
#include <windows.h>
#endif

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include <libavutil/time.h>
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

#define _USE_STL_LOCK_
#ifdef _USE_STL_LOCK_
    #include <mutex>
#endif
struct _graud_lock_ : public std::mutex{
    _graud_lock_(){


//        InitializeCriticalSection(&m_mutex);
#ifndef _USE_STL_LOCK_
        InitializeCriticalSectionAndSpinCount(&m_mutex, 1);
#endif
    }

    virtual ~_graud_lock_(){
#ifndef _USE_STL_LOCK_
        DeleteCriticalSection(&m_mutex);
#endif
    }
#ifdef _USE_STL_LOCK_
//    std::mutex mutex;

#else
    CRITICAL_SECTION m_mutex;
#endif
    void lock(){
#ifdef _USE_STL_LOCK_
//        lk.lock();
#else
        EnterCriticalSection(&m_mutex);
#endif
    }

    void unlock(){
#ifdef _USE_STL_LOCK_
//        lk.unlock();
#else
        LeaveCriticalSection(&m_mutex);
#endif
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

    bool empty(){
        return m_packets.empty();
    }

    void clear(){
        LOCK(mutex)
//        mutex.lock();
        for(auto& it : m_packets){
            av_packet_unref(&it);
        }
//        mutex.unlock();
    }

    bool isMax(){
        return m_maxSize <= m_packets.size();
    }

    bool push_back(const char* msg){
        if(isMax()) return false;
        AVPacket pk;
        av_new_packet(&pk, strlen(msg) + 1);
        strcpy((char*)pk.data, msg);
        return push_back(pk);
    }

    bool push_back(const AVPacket& pk){
        if(isMax()) return false;
        LOCK(mutex)
//        mutex.lock();
        m_packets.push_back(pk);
//        mutex.unlock();
        return true;
    }

    AVPacket pop_front(){
        LOCK(mutex)
//        mutex.lock();
        auto pk = m_packets.front();
        m_packets.pop_front();
//        mutex.unlock();
        return std::move(pk);
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

#define SELECT_PIX_FMT AV_PIX_FMT_YUV420P
    void setsrcCodec(AVCodecContext* codec)
    {
        srcCodec = codec;
        init();
    }

    void init(int width = 0, int height = 0){
        LOCK(mutex)
//        mutex.lock();
        Log(Log_Info, "buffer0:%p, w=%d, h=%d", buffer, w, h);
        uninit();
        if(width) w = width;
        if(height) h = height;

        if(!srcCodec) {
//            mutex.unlock();
            return;
        }

        if(!w) w = srcCodec->width;
        if(!h) h = srcCodec->height;

        frame = av_frame_alloc();
        swsCov = sws_getContext(srcCodec->width, srcCodec->height, srcCodec->pix_fmt
                       , w, h, SELECT_PIX_FMT
                       , SWS_BICUBIC, nullptr, nullptr, nullptr);
        size = avpicture_get_size(SELECT_PIX_FMT, w, h);
        buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
        avpicture_fill((AVPicture*)frame, buffer,SELECT_PIX_FMT, w, h);
//        mutex.unlock();
        Log(Log_Info, "buffer1:%p, w=%d, h=%d", buffer, w, h);
    }

    void uninit()
    {
        if(frame){
            av_free(frame);
            frame = nullptr;
        }

        if(buffer){
            av_free(buffer);
            buffer = nullptr;
        }

        if(swsCov){
            sws_freeContext(swsCov);
            swsCov = nullptr;
        }
    }

    void scale(AVFrame* src, void* cb){
        if(!cb) return;
//        LOCK(mutex)
        auto id = std::this_thread::get_id();
        sws_scale(swsCov, (const uint8_t *const *)src->data
                  ,src->linesize, 0, srcCodec->height, frame->data
                  ,frame->linesize);
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
    _sdl_op_(_audio_sample_* sample, cb callback, void* userdata)
        :nAudioId(-1)
        ,nBuffIndex(0)
        ,nBuffSize(0)
        ,fVolPercent(1)
        ,swrCtx(nullptr)
    {
        target.channels = sample->channels;
        target.freq = sample->rate;
        target.format = AUDIO_S16SYS;
        target.silence = 0;
        target.samples = 1024;
        target.callback = callback;
        target.userdata = userdata;
    }

    ~_sdl_op_()
    {
//        /*if(!nInit)*/ SDL_Quit();
        if(swrCtx){
            swr_free(&swrCtx);
            swrCtx = nullptr;
        }
    }

    int init(){
        /*static*/ int nInit = SDL_Init(SDL_INIT_AUDIO);
        return nInit;
    }

    int open(){
        auto num = SDL_GetNumAudioDevices(0);
        for(int i = 0; i < num; ++i){
            auto name = SDL_GetAudioDeviceName(i, 0);
            Log(Log_Info, "audio device %d=%s!", i, name);
            nAudioId = SDL_OpenAudioDevice(name, false, &target, &spec, 0);
            if(nAudioId > 0) return nAudioId;
        }

        Log(Log_Warning, "%s!", SDL_GetError());
        return -1;
    }

    void close(){
        if(nAudioId > 0){
            SDL_LockAudioDevice(nAudioId);
            SDL_PauseAudioDevice(nAudioId, 1);
            SDL_UnlockAudioDevice(nAudioId);
            SDL_CloseAudioDevice(nAudioId);
            nAudioId = -1;
        }
    }

    void start(){
        SDL_LockAudioDevice(nAudioId);
        SDL_PauseAudioDevice(nAudioId, 0);
        SDL_UnlockAudioDevice(nAudioId);
    }

    // adjust volume
    void adjustVol(int size){
        int uRepeat = 1; // repeat times
        auto buf = (char*)buff + nBuffIndex;
        if (!size) return;
        for (int i = 0; i < size; i += 2)
        {
            short wData;
            wData = MAKEWORD(buf[i], buf[i + 1]);
            long dwData = wData;
            for (int j = 0; j < uRepeat; j++)
            {
                dwData = dwData * fVolPercent;
                if (dwData < -0x8000)
                {
                    dwData = -0x8000;
                }
                else if (dwData > 0x7FFF)
                {
                    dwData = 0x7FFF;
                }
            }
            wData = LOWORD(dwData);
            buf[i] = LOBYTE(wData);
            buf[i + 1] = HIBYTE(wData);
        }
    }

    int nAudioId;
    _audio_sample_ in;
    _audio_sample_ out;
    SDL_AudioSpec spec;
    SDL_AudioSpec target;
    DECLARE_ALIGNED(16,uint8_t,buff) [192000 * 4];
    unsigned int nBuffIndex;
    unsigned int nBuffSize;
    float fVolPercent;
    SwrContext *swrCtx;
};

struct _ffmpeg_audio_info_
{
    _ffmpeg_audio_info_()
        :pCodec(nullptr)
        ,pFrame(nullptr)
        ,pFrameReSample(nullptr)
        ,_clock(0)
        ,nStreamIndex(-1)
        ,sdl(nullptr)
    {

    }

    ~_ffmpeg_audio_info_()
    {
        if(pFrame){
            av_frame_free(&pFrame);
            pFrame = nullptr;
        }
        if(pFrameReSample){
            av_frame_free(&pFrameReSample);
            pFrameReSample = nullptr;
        }

        if(sdl){
            delete sdl;
            sdl = nullptr;
        }
    }
    AVCodec* pCodec;
    AVFrame* pFrame;
    AVFrame* pFrameReSample;
    _ffmpeg_packets_ pks;
    double _clock;
    int nStreamIndex;
    _sdl_op_* sdl;
};

#define CALC_PTS(x, y) ((x - y) / 1000000.0)
#define CALC_DELAY(x, y) ((x) > (y) ? (y) : (x))

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

inline bool isSetBit(unsigned int flag, int bit){
    return ((flag >> bit) & 1) == 1;
}

inline void setBit(unsigned int& flag, int bit, bool value = true){
    if(!value)
        flag &= ~(1 << bit);
    else
        flag |= (1 << bit);
}

struct _video_info_
{
    _video_info_()
        :state(0)
        ,_start_time(0)
        ,_seek_time(0)
        ,_seek_pos(0)
        ,_cb(nullptr)
        ,_flag(0)
        ,_format_ctx(nullptr)
    {

    }

    virtual ~_video_info_()
    {
        if(_format_ctx){
            avformat_close_input(&_format_ctx);
            avformat_free_context(_format_ctx);
            _format_ctx = nullptr;
        }
    }
    int state;
    std::string src;
    double _start_time, _seek_time;
    int64_t _seek_pos;
    void* _cb;
    unsigned int _flag;
    AVFormatContext* _format_ctx;
    _ffmpeg_out_frame_ yuv;
    _ffmpeg_video_info_ video;
    _ffmpeg_audio_info_ audio;
};

#endif // VIDEO_DEFINE_H
