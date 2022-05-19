#include "core_dev_xaudio2.h"
#include "core_dev_p.h"
#include <xaudio2.h>

struct StreamingVoiceContext : public IXAudio2VoiceCallback
{
    STDMETHOD_( void, OnVoiceProcessingPassStart )( UINT32 )
    {
    }
    STDMETHOD_( void, OnVoiceProcessingPassEnd )()
    {
    }
    STDMETHOD_( void, OnStreamEnd )()
    {
    }
    STDMETHOD_( void, OnBufferStart )( void* )
    {
    }
    STDMETHOD_( void, OnBufferEnd )( void* )
    {
        SetEvent( hBufferEndEvent );
    }
    STDMETHOD_( void, OnLoopEnd )( void* )
    {
    }
    STDMETHOD_( void, OnVoiceError )( void*, HRESULT )
    {
    }

    HANDLE hBufferEndEvent;

    StreamingVoiceContext() : hBufferEndEvent( CreateEvent( nullptr, FALSE, FALSE, nullptr ) )
    {
    }
    virtual ~StreamingVoiceContext()
    {
        CloseHandle( hBufferEndEvent );
    }
};

class core_dev_xaudio2Private : public core_devPrivate
{
    VP_DECLARE_PUBLIC(core_dev_xaudio2)

    inline core_dev_xaudio2Private(core_dev_xaudio2* parent)
        : core_devPrivate(parent)
        , m_engine(nullptr)
        , m_voiceMaster(nullptr)
        , m_voiceSrc(nullptr)
    {
    }

    bool initXAduio2()
    {
        if(!m_engine)
        {
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if(FAILED(XAudio2Create(&m_engine))) return false;
            if(FAILED(m_engine->CreateMasteringVoice(&m_voiceMaster))) return false;
            if(FAILED(m_engine->CreateSourceVoice(&m_voiceSrc, &m_fmt, 0, 1.0f, &m_context))) return false;
        }

        return true;
    }

    void uninitXAduio2()
    {
        m_voiceMaster = nullptr;
        m_voiceSrc = nullptr;
        SAFE_RELEASE(m_engine)
        CoUninitialize();
    }

    void setWaveInfo(DWORD sampleRate, WORD channels, WORD Bits)
    {
        Log(Log_Debug, "samples=%d channels=%d, bits=%d", sampleRate, channels, Bits);
        //采样率
        m_fmt.nSamplesPerSec = sampleRate;
        //声道数
        m_fmt.nChannels = channels;
        //采样精度
        m_fmt.wBitsPerSample = Bits;
        //数据流速度（Byte/s） = 采样率×声道数×采样精度/8
        m_fmt.nAvgBytesPerSec = m_fmt.nSamplesPerSec * m_fmt.nChannels * m_fmt.wBitsPerSample / 8;
        //数据块大小（单位样本所占的字节数） = 声道数×采样精度/8
        m_fmt.nBlockAlign = m_fmt.nChannels * m_fmt.wBitsPerSample / 8;
        //额外信息
        m_fmt.cbSize = 0;
        //PCM格式
        m_fmt.wFormatTag = WAVE_FORMAT_PCM;
    }

    void stop() override
    {
        core_devPrivate::stop();
        uninitXAduio2();
    }

    void start() override
    {
        core_devPrivate::start();
        Log(Log_Debug, "%d", m_state);
        if(m_state == audio_running && m_voiceSrc) m_voiceSrc->Start(0, 0);
    }

    void pause() override
    {
        if(m_state == audio_pause)
            return;
        core_devPrivate::pause();
        if(m_voiceSrc) m_voiceSrc->Stop();
    }

    void cleanBuffer() override
    {
        core_devPrivate::cleanBuffer();
        if(m_voiceSrc) m_voiceSrc->FlushSourceBuffers();
    }

    bool init(AVCodecContext *ctx, const core_audio_sample &sample) override
    {
        if(m_engine)
            return true;

        initSample(ctx);

        setWaveInfo(static_cast<DWORD>(sample.rate), static_cast<WORD>(sample.channels), 16);

        if(!initXAduio2())
            return false;

        initThread();

        m_voiceSrc->Start(0, 0);

        return true;
    }

    void play() override
    {
        memset(&m_voiceBuf, 0x00, sizeof(m_voiceBuf));
        m_voiceBuf.pAudioData = m_buffer + m_pos;
        m_voiceBuf.AudioBytes = m_frameSize;
        auto hr = m_voiceSrc->SubmitSourceBuffer(&m_voiceBuf);
//        XAUDIO2_MAX_QUEUED_BUFFERS
        if(FAILED(hr))
        {
            Log(Log_Debug, "m_read=%-7d len=%-7d index=%-7d hr=%p", m_read, m_frameSize, m_pos, hr);
            return;
        }

        m_read -= m_frameSize;
        m_pos += m_frameSize;
        m_pos %= m_frameSize * m_frameCount;
    }

    bool waitPlay() override
    {
        if(!core_devPrivate::waitPlay())
            return false;

        m_voiceSrc->GetState(&m_voiceState);
        if(m_voiceState.BuffersQueued > m_frameCount - 1)
        {
            WaitForSingleObject(m_context.hBufferEndEvent, INFINITE);
            return false;
        }

        return true;
    }

    IXAudio2                *m_engine;
    IXAudio2MasteringVoice  *m_voiceMaster;
    IXAudio2SourceVoice     *m_voiceSrc;
    StreamingVoiceContext   m_context;
    XAUDIO2_BUFFER          m_voiceBuf;
    WAVEFORMATEX            m_fmt;
    XAUDIO2_VOICE_STATE     m_voiceState;
};

core_dev_xaudio2::core_dev_xaudio2()
    : core_dev(new core_dev_xaudio2Private(this))
{

}
