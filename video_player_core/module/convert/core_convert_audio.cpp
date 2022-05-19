#include "core_convert_audio.h"
#include "core_convert_p.h"
#include <fftreal_wrapper.h>

#include "video_player_core.h"
#include "../common.h"
#include "../sample/core_audio_sample.h"
#include "../dev/core_dev_sdl.h"

#ifdef AUDIO_DEV_SELECT
#include "../dev/core_dev_waveout.h"
#include "../dev/core_dev_dsound.h"
#include "../dev/core_dev_xaudio2.h"
#endif

class core_convert_audioPrivate : public core_convertPrivate
{
    VP_DECLARE_PUBLIC(core_convert_audio)
    inline core_convert_audioPrivate(core_convert_audio *parent)
        : core_convertPrivate(parent)
        , m_spectrumIndex(0)
        , m_fVolPercent(0.5f)
        , m_nVol(SDL_MIX_MAXVOLUME / 2)
        , m_audioChannelType(audio_channel_both)
    {
//        init();
    }

    ~core_convert_audioPrivate() override
    {
        if(m_buffer)
        {
            delete[] m_buffer;
            m_buffer = nullptr;
        }

        dev()->stop();
    }

    void init()
    {
        initFrame();
        m_fft           = new FFTRealWrapper();
        m_codecCtx      = nullptr;
        m_fVolPercent   = 0.5f;
        m_nVol          = SDL_MIX_MAXVOLUME / 2;
        m_audioChannelType = audio_channel_both;
        m_spectrumIndex = 0;
        m_swrCtx        = nullptr;
        m_buffer        = new uint8_t[192000 * 4];
    }

    bool initSwrCtx()
    {
        unInitSwrCtx();
        m_swrCtx = swr_alloc_set_opts(
            nullptr,
            static_cast<int64_t>(m_out.layout), m_out.fmt, m_out.rate,
            static_cast<int64_t>(m_in.layout), m_in.fmt, m_in.rate,
            0,
            nullptr);
        int nRet = 0;
        if((nRet = swr_init(m_swrCtx)) < 0)
        {
            char err[128] = {};
            av_strerror(nRet, err, 128);
            Log(Log_Warning, "open resample failed, %s", err);
            unInitSwrCtx();
            return false;
        }

        return true;
    }

    void unInitSwrCtx()
    {
        if(m_swrCtx)
        {
            swr_free(&m_swrCtx);
            m_swrCtx = nullptr;
        }
    }

    bool initResample() override
    {
        m_in = core_audio_sample(m_codecCtx);
        return initSwrCtx();
    }

    bool initDev();

    void formatChannelType(Uint8* buff, unsigned int size)
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

    bool formatFreq(Uint8 *buff, unsigned int size, video_interface *cb);

    bool convert(AVFrame *) override;

    core_dev *dev() override;

    void reset() override
    {
        m_spectrumIndex = 0;
        memset(m_buffer, 0x00, 192000 * 4);
    }

    void setVolume(int vol) override
    {
        m_nVol = vol;
    }

    int getVolume() override
    {
        return m_nVol;
    }

    void setChannelType(int type) override
    {
        m_audioChannelType = type;
    }

    static core_dev* dev_init()
    {
#ifdef AUDIO_DEV_SELECT
        return new core_dev_sdl;
//        return new core_dev_waveout;
//        return new core_dev_dsound;
//        return new core_dev_xaudio2;
#else
        return new core_dev_sdl;
#endif
    }

    core_audio_sample   m_in;
    core_audio_sample   m_out;
    SDL_AudioSpec       m_spec;
    unsigned int        m_spectrumIndex;
    float               m_fVolPercent;
    int                 m_nVol;
    int                 m_audioChannelType;
#define FREQ_POINT 16384
    FFTRealWrapper      *m_fft;
    char                m_freq[FREQ_POINT];
    SwrContext          *m_swrCtx;
    static core_dev     *m_dev;
    friend class core_thread_audio;
};

core_dev* core_convert_audioPrivate::m_dev = core_convert_audioPrivate::dev_init();
bool core_convert_audioPrivate::initDev()
{
    return m_dev->init(m_codecCtx, m_out);
}

bool core_convert_audioPrivate::formatFreq(Uint8 *buff, unsigned int size, video_interface *cb)
{
    if(m_spectrumIndex + size >= FREQ_POINT)
    {
        memcpy(m_freq + m_spectrumIndex, buff, FREQ_POINT - m_spectrumIndex);
        size -= (FREQ_POINT - m_spectrumIndex);
        //fft
        unsigned int samples = FREQ_POINT / 4;
        std::vector<float> input(samples);
        std::vector<float> output(samples, 0);
        auto pSpec = reinterpret_cast<short*>(m_freq);
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
        memcpy(static_cast<char*>(m_freq) + m_spectrumIndex, buff, size);
        m_spectrumIndex += size;
    }

    return false;
}

bool core_convert_audioPrivate::convert(AVFrame *src)
{
#ifdef AUDIO_FILTER
    const unsigned int n = static_cast<unsigned int>(src->nb_samples * av_get_channel_layout_nb_channels(src->channel_layout) * src->channels);
    const uint8_t *p = src->data[0];
    memcpy(m_buffer, p, n);
    m_size = n;
#else
    if(m_frame->nb_samples != src->nb_samples)
    {
        auto delay = swr_get_delay(m_swrCtx, m_out.rate) + src->nb_samples;
        m_frame->nb_samples = static_cast<int>(av_rescale_rnd(
            delay, m_out.rate, m_in.rate, AV_ROUND_UP));
        av_samples_fill_arrays(m_frame->data,
                               m_frame->linesize,
                               m_buffer,
                               m_out.channels,
                               m_frame->nb_samples,
                               m_out.fmt,
                               0);
    }

    auto len = swr_convert(m_swrCtx, m_frame->data, m_frame->nb_samples,
                           const_cast<const uint8_t**>(src->data), src->nb_samples);
    m_size = static_cast<unsigned int>(len * m_out.channels * av_get_bytes_per_sample(m_out.fmt));
#endif
    return true;
}

core_dev *core_convert_audioPrivate::dev()
{
    return m_dev;
}

core_convert_audio::core_convert_audio()
    : core_convert(new core_convert_audioPrivate(this))
{
}

core_convert_audio::~core_convert_audio()
{
    Log(Log_Info, "");
}

void core_convert_audio::setContext(AVCodecContext *ctx)
{
    VP_D(core_convert_audio);
    d->init();
    core_convert::setContext(ctx);
    d->initDev();
}

void core_convert_audio::formatChannelType(Uint8 *buff, unsigned int size)
{
    VP_D(core_convert_audio);
    d->formatChannelType(buff, size);
}

bool core_convert_audio::formatFreq(Uint8 *buff, unsigned int size, video_interface *cb)
{
    VP_D(core_convert_audio);
    return d->formatFreq(buff, size, cb);
}


