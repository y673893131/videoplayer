#include "core_dev_sdl.h"
#include "core_dev_p.h"
#include "../common.h"
#include "../convert/core_convert_audio.h"
#include "../sample/core_audio_sample.h"

class core_dev_sdlPrivate : public core_devPrivate
{
    VP_DECLARE_PUBLIC(core_dev_sdl)
    inline core_dev_sdlPrivate(core_dev_sdl* parent)
        : core_devPrivate(parent)
    {
    }

    bool init(AVCodecContext* ctx, const core_audio_sample& sample) override
    {
        int nRet = 0;
        if(SDL_WasInit(SDL_INIT_AUDIO))
        {
            return true;
        }

        SetEnvironmentVariable(L"SDL_AUDIODRIVER", L"directsound");

        if((nRet = initSDL()))
        {
            Log(Log_Warning, "sdl init failed, %d", nRet);
            return false;
        }

        initSample(ctx);

        m_target.samples    = static_cast<Uint16>(m_samples);
        m_target.channels   = static_cast<Uint8>(sample.channels);
        m_target.freq       = sample.rate;
        m_target.format     = AUDIO_S16SYS;
        m_target.silence    = 0;
        m_target.callback   = m_callback;
        m_target.userdata   = m_userData;

        if(openSDL() < 0)
            return false;

        return true;
    }

    void start() override;
    void stop() override
    {
    }
    void pause() override;
    void reset() override
    {
    }

    void append(uint8_t *dst, uint8_t *src, unsigned int len, int vol) override
    {
        vol = vol > SDL_MIX_MAXVOLUME ? SDL_MIX_MAXVOLUME : vol;
        SDL_MixAudio(dst, src, len, vol);
    }

    bool waitPlay() override
    {
        msleep(10);
        return false;
    }

    bool waitDecode() override
    {
        msleep(10);
        return false;
    }

    void play() override
    {
    }

    bool checkSDL();
    int initSDL();
    int openSDL();
    void closeSDL();

    int             m_nAudioId;
    SDL_AudioSpec   m_target;
};

bool core_dev_sdlPrivate::checkSDL()
{
    return m_nAudioId >= 0;
}

int core_dev_sdlPrivate::initSDL()
{
    /*static*/ int nInit = SDL_Init(SDL_INIT_AUDIO);
    return nInit;
}

int core_dev_sdlPrivate::openSDL()
{
    int ret = SDL_OpenAudio(&m_target, nullptr);
    if (ret<0)
    {
        Log(Log_Err, "sdl open audio device failed. %s", SDL_GetError());
        return -1;
    }
    else
    {
        return 0;
    }

//    auto num = SDL_GetNumAudioDevices(0);
//    for(int i = 0; i <= num; ++i)
//    {
//        auto name = SDL_GetAudioDeviceName(i, 0);
//        Log(Log_Info, "audio device name[%d]=%s start!", i, name);
//        nAudioId = SDL_OpenAudioDevice(name, false, &target, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
//        Log(Log_Info, "audio device id=%d name%d=%s!", nAudioId, i, name);
//        if(nAudioId > 0) return nAudioId;
//    }

//    Log(Log_Warning, "sdl open failed, audio devices count:%d, %s", num, SDL_GetError());
//    return -1;
}

void core_dev_sdlPrivate::closeSDL()
{
    Log(Log_Info, "sdl close:%d, thread:%d", m_nAudioId, std::this_thread::get_id());
    if(m_nAudioId > 0)
    {
        Log(Log_Info, "sdl close:%d, thread:%d, 0x%p", m_nAudioId, std::this_thread::get_id(), this);
//        SDL_LockAudioDevice(nAudioId);
//        SDL_CloseAudioDevice(nAudioId);
//        SDL_UnlockAudioDevice(nAudioId);

        SDL_LockAudio();
        SDL_CloseAudio();
        SDL_UnlockAudio();
        Log(Log_Info, "sdl close:successed, thread: %d", std::this_thread::get_id());
        m_nAudioId = -1;
    }
}

void core_dev_sdlPrivate::start()
{
    Log(Log_Debug, "start SDL");
    SDL_AudioStatus state = SDL_GetAudioStatus();
    if(state != SDL_AUDIO_PLAYING)
    {
        SDL_LockAudio();
        SDL_PauseAudio(0);
        SDL_UnlockAudio();
    }
//    if(nAudioId > 0)
//    {
//        SDL_LockAudioDevice(nAudioId);
//        SDL_PauseAudioDevice(nAudioId, 0);
//        SDL_UnlockAudioDevice(nAudioId);
//    }
}

void core_dev_sdlPrivate::pause()
{
    Log(Log_Debug, "pause SDL");
    SDL_AudioStatus state = SDL_GetAudioStatus();
    if(state == SDL_AUDIO_PLAYING)
    {
        SDL_LockAudio();
        SDL_PauseAudio(1);
        SDL_UnlockAudio();
    }
//    if(nAudioId > 0)
//    {
//        SDL_LockAudioDevice(nAudioId);
//        SDL_PauseAudioDevice(nAudioId, 1);
//        SDL_UnlockAudioDevice(nAudioId);
    //    }
}

core_dev_sdl::core_dev_sdl()
    : core_dev(new core_dev_sdlPrivate(this))
{
}
