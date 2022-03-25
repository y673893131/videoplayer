#include "core_sdl_op.h"
#include "video_player_core.h"
#include <fftreal_wrapper.h>

core_sdl_op::core_sdl_op()
    :nAudioId(-1)
    ,nBuffIndex(0)
    ,nBuffSize(0)
    ,fVolPercent(0.5f)
    ,nVol(SDL_MIX_MAXVOLUME / 2)
    ,m_audioChannelType(audio_channel_both)
    ,m_spectrumIndex(0)
    ,swrCtx(nullptr)
{
    pFrameReSample = av_frame_alloc();
    m_fft = new FFTRealWrapper();
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

    if(m_fft)
    {
        delete m_fft;
        m_fft = nullptr;
    }
}

void core_sdl_op::resetSpec()
{
    m_spectrumIndex = 0;
}

void core_sdl_op::setCodecContext(AVCodecContext *ctx)
{
    decodectx = ctx;
}

bool core_sdl_op::initResample()
{
    if(swrCtx)
    {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }

    // resample: in->out
    in = core_audio_sample(decodectx);
    swrCtx = swr_alloc_set_opts(
                nullptr,
                static_cast<int64_t>(out.layout), out.fmt, out.rate,
                static_cast<int64_t>(in.layout), in.fmt, in.rate,
                0,
                nullptr);
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
    if(SDL_WasInit(SDL_INIT_AUDIO))
    {
        return true;
    }

    if((nRet = initSDL()))
    {
        Log(Log_Warning, "sdl init failed, %d", nRet);
        return false;
    }

#ifdef AUDIO_FILTER
    target.samples = static_cast<Uint16>(decodectx->frame_size);
#else
    target.samples = 1024;
#endif

    target.channels = static_cast<Uint8>(out.channels);
    target.freq = out.rate;
    target.format = AUDIO_S16SYS;
    target.silence = 0;
    target.callback = callback;
    target.userdata = userdata;

    if(openSDL() < 0)
        return false;

    return true;
}

#ifdef WIN32
#include <Windows.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#define Release_Com_Obj(obj) if ((obj)) (obj)->Release();
#pragma comment(lib,"ole32.lib")

#define Check_HR_Break(hr, str)         if ((hr) != S_OK)\
        {\
        printf((str));\
        break;\
        }

// 设置程序自身音量，范围 0-1.0。与 C:\Windows\System32\sndvol.exe 一致
//void TestSimpleAudioVolume(int iNum)
//{
//    HRESULT hr = CoInitialize(nullptr);
//    if (FAILED(hr))
//    {
//        CoUninitialize();
//        return;
//    }

//    IAudioClient* pAudioClient = NULL;
//    IMMDeviceEnumerator* pDeviceEnumerator = NULL;
//    IMMDevice* pDevice = NULL;
//    ISimpleAudioVolume* pVolume = NULL;

//    do
//    {
//        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pDeviceEnumerator);

//        Check_HR_Break(hr, "CoCreateInstance Failed\n");

//        hr = pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
//        Check_HR_Break(hr, "GetDefaultAudioEndpoint Failed\n");

//        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
//        Check_HR_Break(hr, "Activate Failed\n");

//        float fVolumeLevel = 0;

//        REFERENCE_TIME hnsRequestedDuration = 100;
//        WAVEFORMATEX* pwfx = NULL;

//        hr = pAudioClient->GetMixFormat(&pwfx);
//        Check_HR_Break(hr, "GetMixFormat Failed\n");

//        hr = pAudioClient->Initialize(
//            AUDCLNT_SHAREMODE_SHARED,
//            0,
//            hnsRequestedDuration,
//            0,
//            pwfx,
//            NULL);
//        Check_HR_Break(hr, "Initialize Failed\n");

//        hr = pAudioClient->GetService(__uuidof(ISimpleAudioVolume), (void**)&pVolume);
//        Check_HR_Break(hr, "GetService Failed\n");


//        hr = pVolume->GetMasterVolume(&fVolumeLevel);
//        Check_HR_Break(hr, "GetMasterVolume Failed\n");
//        printf("GetMasterVolume level = %f\n", fVolumeLevel);

//        hr = pVolume->SetMasterVolume(iNum / 100.0f, NULL);
//        Check_HR_Break(hr, "SetMasterVolume Failed\n");

//        hr = pVolume->GetMasterVolume(&fVolumeLevel);
//        Check_HR_Break(hr, "GetMasterVolume Failed\n");
//        printf("GetMasterVolume level = %f\n", fVolumeLevel);
//        printf("\n");
//    } while (0);


//    Release_Com_Obj(pAudioClient);
//    Release_Com_Obj(pDeviceEnumerator);
//    Release_Com_Obj(pDevice);
//    Release_Com_Obj(pVolume);

//    CoUninitialize();
//}
#endif

void core_sdl_op::setVol(int vol)
{
    fVolPercent = static_cast<float>(vol);
    nVol = static_cast<int>(vol / 100.0f * SDL_MIX_MAXVOLUME);
//    TestSimpleAudioVolume(nVol);
}

int core_sdl_op::getVol()
{
    return static_cast<int>(fVolPercent);
}

void core_sdl_op::setChannelType(int type)
{
    m_audioChannelType = type;
}

void core_sdl_op::formatChannelType(Uint8* buff, unsigned int size, video_interface* cb)
{
    switch (m_audioChannelType) {
    case audio_channel_both:
        break;
    case audio_channel_left:
        for(unsigned int i = 0; i < size; i += 4)
        {
            buff[i + 2] = buff[i];
            buff[i + 3] = buff[i + 1];
        }
        break;
    case audio_channel_right:
        for(unsigned int i = 0; i < size; i += 4)
        {
            buff[i] = buff[i + 2];
            buff[i + 1] = buff[i + 3];
        }
        break;
    }
}

bool core_sdl_op::formatFreq(Uint8 *buff, unsigned int size, video_interface *cb)
{
    if(m_spectrumIndex + size >= FREQ_POINT)
    {
        memcpy(m_spec + m_spectrumIndex, buff, FREQ_POINT - m_spectrumIndex);
        size -= (FREQ_POINT - m_spectrumIndex);
        //fft
        unsigned int samples = FREQ_POINT / 4;
        std::vector<float> input(samples);
        std::vector<float> output(samples, 0);
        auto pSpec = reinterpret_cast<short*>(m_spec);
        for (unsigned int n = 0; n < samples; ++n) {
            auto temp = pSpec[n] / 32768.0f;
            input[n] = temp;
        }

        m_fft->calculateFFT(output.data(), input.data());

        auto count = samples / 2;

#define FREQ_BAR_COUNT 128
#define LOW_FREQ 0.0
#define HIGH_FREQ 2000.0
#define INPUT_FREQ 44100
        static float bar[FREQ_BAR_COUNT] = {};
        memset(bar, 0x00, sizeof(bar));
        for(unsigned int i = 2; i <= count; ++i){

            auto real = output[i];
            float imag = 0;
            if(i != count)
                imag = output[count + i];
            auto magnitude = std::sqrt(real * real + imag * imag);
            auto ap = 0.15f * std::log(magnitude);
            if(ap > 1.0f)
                ap = 1.0f;
            else if(ap < 0.0f)
                ap = 0.0f;

            auto freq = 1.0 * i * INPUT_FREQ / samples;
            if(freq >= LOW_FREQ && freq < HIGH_FREQ)
            {
                auto bandwidth = (HIGH_FREQ - LOW_FREQ) / FREQ_BAR_COUNT;
                unsigned int index = static_cast<unsigned int>((freq - LOW_FREQ) / bandwidth);
                if(bar[index] < ap)
                    bar[index] = ap;
            }
        }

        cb->displayFreqCall(bar, FREQ_BAR_COUNT);

//        LogB(Log_Debug, "%d", size);
        m_spectrumIndex = 0;
        return true;
    }
    else
    {
        memcpy(static_cast<char*>(m_spec) + m_spectrumIndex, buff, size);
        m_spectrumIndex += size;
        return false;
    }
}

void core_sdl_op::resampleFrame(AVFrame *frame, unsigned int& bufferSize)
{
    checkSDL();
#ifdef AUDIO_FILTER
    const unsigned int n = static_cast<unsigned int>(frame->nb_samples * av_get_channel_layout_nb_channels(frame->channel_layout) * frame->channels);
    const uint8_t *p = frame->data[0];
    memcpy(buff, p, n);
    bufferSize = n;
#else
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
                           const_cast<const uint8_t**>(frame->data), frame->nb_samples);
    bufferSize = static_cast<unsigned int>(len * target.channels * av_get_bytes_per_sample(out.fmt));
#endif
    return;
}

bool core_sdl_op::checkSDL()
{
    return nAudioId >= 0;
}

int core_sdl_op::initSDL()
{
    /*static*/ int nInit = SDL_Init(SDL_INIT_AUDIO);
    return nInit;
}

int core_sdl_op::openSDL()
{
    int ret = SDL_OpenAudio(&target, nullptr);
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

void core_sdl_op::closeSDL()
{
    Log(Log_Info, "sdl close:%d, thread:%d", nAudioId, std::this_thread::get_id());
    if(nAudioId > 0)
    {
        Log(Log_Info, "sdl close:%d, thread:%d, 0x%p", nAudioId, std::this_thread::get_id(), this);
//        SDL_LockAudioDevice(nAudioId);
//        SDL_CloseAudioDevice(nAudioId);
//        SDL_UnlockAudioDevice(nAudioId);

        SDL_LockAudio();
        SDL_CloseAudio();
        SDL_UnlockAudio();
        Log(Log_Info, "sdl close:successed, thread: %d", std::this_thread::get_id());
        nAudioId = -1;
    }
}

void core_sdl_op::startSDL()
{
//    Log(Log_Debug, "start SDL");
    SDL_LockAudio();
    SDL_PauseAudio(0);
    SDL_UnlockAudio();
//    if(nAudioId > 0)
//    {
//        SDL_LockAudioDevice(nAudioId);
//        SDL_PauseAudioDevice(nAudioId, 0);
//        SDL_UnlockAudioDevice(nAudioId);
//    }
}

void core_sdl_op::pauseSDL()
{
//    Log(Log_Debug, "pause SDL");
    SDL_LockAudio();
    SDL_PauseAudio(1);
    SDL_UnlockAudio();
//    if(nAudioId > 0)
//    {
//        SDL_LockAudioDevice(nAudioId);
//        SDL_PauseAudioDevice(nAudioId, 1);
//        SDL_UnlockAudioDevice(nAudioId);
    //    }
}
