#include "core_sdl_op.h"

core_sdl_op::core_sdl_op()
    :nAudioId(-1)
    ,nBuffIndex(0)
    ,nBuffSize(0)
    ,fVolPercent(1)
    ,nVol(SDL_MIX_MAXVOLUME)
    ,swrCtx(nullptr)
{
    pFrameReSample = av_frame_alloc();
}

core_sdl_op::~core_sdl_op()
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
    if(pFrameReSample)
    {
        av_frame_free(&pFrameReSample);
        pFrameReSample = nullptr;
    }
}

bool core_sdl_op::initResample(AVCodecContext* ctx)
{
    if(swrCtx)
    {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }

    // resample: in->out
    in = core_audio_sample(ctx);
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

bool core_sdl_op::init(audioCallback callback, void* userdata)
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

void core_sdl_op::setVol(int vol)
{
    fVolPercent = static_cast<float>(vol);
    nVol = static_cast<int>(fVolPercent / 100 * SDL_MIX_MAXVOLUME);
}

float core_sdl_op::getVol()
{
    return fVolPercent;
}

void core_sdl_op::resampleFrame(AVFrame *frame, int& bufferSize)
{
    if(pFrameReSample->nb_samples != frame->nb_samples)
    {
        auto delay = swr_get_delay(swrCtx, out.rate) + frame->nb_samples;

        pFrameReSample->nb_samples = static_cast<int>(av_rescale_rnd(
            delay, out.rate, in.rate, AV_ROUND_UP));
        av_samples_fill_arrays(pFrameReSample->data, pFrameReSample->linesize,
                               buff, target.channels, pFrameReSample->nb_samples,
                               out.fmt, 0);
    }

    auto len = swr_convert(swrCtx, pFrameReSample->data, pFrameReSample->nb_samples,
                           (const uint8_t**)frame->data, frame->nb_samples);
    bufferSize = len * target.channels * av_get_bytes_per_sample(out.fmt);
    return;
}

int core_sdl_op::initSDL()
{
    /*static*/ int nInit = SDL_Init(SDL_INIT_AUDIO);
    return nInit;
}

int core_sdl_op::openSDL()
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

void core_sdl_op::closeSDL()
{
    Log(Log_Info, "sdl close:%d, thread:%d", nAudioId, std::this_thread::get_id());
    if(nAudioId > 0)
    {
        Log(Log_Info, "sdl close:%d, thread:%d, 0x%p", nAudioId, std::this_thread::get_id(), this);
        SDL_LockAudioDevice(nAudioId);
        SDL_CloseAudioDevice(nAudioId);
        SDL_UnlockAudioDevice(nAudioId);
        Log(Log_Info, "sdl close:successed, thread: %d", std::this_thread::get_id());
        nAudioId = -1;
    }
}

void core_sdl_op::startSDL()
{
    if(nAudioId > 0)
    {
        SDL_LockAudioDevice(nAudioId);
        SDL_PauseAudioDevice(nAudioId, 0);
        SDL_UnlockAudioDevice(nAudioId);
    }
}

void core_sdl_op::pauseSDL()
{
    if(nAudioId > 0)
    {
        SDL_LockAudioDevice(nAudioId);
        SDL_PauseAudioDevice(nAudioId, 1);
        SDL_UnlockAudioDevice(nAudioId);
    }
}
