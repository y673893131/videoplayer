#include "core_dev_dsound.h"
#include "core_dev_p.h"
#include "../lock/core_graud_lock.h"
#include <dsound.h>

class core_dev_dsoundPrivate : public core_devPrivate
{
    VP_DECLARE_PUBLIC(core_dev_dsound)

    inline core_dev_dsoundPrivate(core_dev_dsound* parent)
        : core_devPrivate(parent)
        , m_ds8(nullptr)
        , m_ds8Buffer(nullptr)
        , m_dsSecondBuffer(nullptr)
    {
        m_ds8PosNotify = new DSBPOSITIONNOTIFY[m_frameCount];
        memset(m_ds8PosNotify, 0x00, sizeof(DSBPOSITIONNOTIFY) * m_frameCount);

        m_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    ~core_dev_dsoundPrivate() override
    {
        SAFE_RELEASE(m_ds8Buffer)
        SAFE_RELEASE(m_dsSecondBuffer)
        SAFE_RELEASE(m_ds8)
        CLOSE_HANDLE(m_event)
    }

    static BOOL CALLBACK find(HWND hwnd, LPARAM lParam)
    {
        auto pParams = reinterpret_cast<std::pair<HWND, DWORD>*>(lParam);

        DWORD processId;
        if (GetWindowThreadProcessId(hwnd, &processId) && processId == pParams->second)
        {
            // Stop enumerating
            pParams->first = hwnd;
            return FALSE;
        }

        // Continue enumerating
        return TRUE;
    }

    HWND FindTopWindow(DWORD pid)
    {
        std::pair<HWND, DWORD> params = { nullptr, pid };

        // Enumerate the windows using a lambda to process each window
        BOOL bResult = EnumWindows(find, reinterpret_cast<LPARAM>(&params));
        if (!bResult && params.first)
        {
            return params.first;
        }

        return nullptr;
    }

    void initDS()
    {
        if(!m_ds8)
        {
            DirectSoundCreate8(nullptr, &m_ds8, nullptr);
            auto pid = ::GetCurrentProcessId();
            auto hwnd = FindTopWindow(pid);
            auto hr = m_ds8->SetCooperativeLevel(hwnd, /*DSSCL_PRIORITY*/DSSCL_NORMAL);
            Log(Log_Debug, "SetCooperativeLevel[%p]", hr);

            DSCAPS dscaps;
            memset(&dscaps, 0x00, sizeof(DSCAPS));
            dscaps.dwSize = sizeof(DSCAPS);
            //DSCAPS_EMULDRIVER
            hr = m_ds8->GetCaps(&dscaps);
            Log(Log_Debug, "dscaps[%p] flag=%p", hr, dscaps.dwFlags);
        }
    }

    void setWaveInfo(DWORD sampleRate, WORD channels, WORD Bits)
    {
        Log(Log_Debug, "samples=%d channels=%d, bits=%d", sampleRate, channels, Bits);
        //采样率
        m_waveFormatex.nSamplesPerSec = sampleRate;
        //声道数
        m_waveFormatex.nChannels = channels;
        //采样精度
        m_waveFormatex.wBitsPerSample = Bits;
        //数据流速度（Byte/s） = 采样率×声道数×采样精度/8
        m_waveFormatex.nAvgBytesPerSec = m_waveFormatex.nSamplesPerSec * m_waveFormatex.nChannels * m_waveFormatex.wBitsPerSample / 8;
        //数据块大小（单位样本所占的字节数） = 声道数×采样精度/8
        m_waveFormatex.nBlockAlign = m_waveFormatex.nChannels * m_waveFormatex.wBitsPerSample / 8;
        //额外信息
        m_waveFormatex.cbSize = 0;
        //PCM格式
        m_waveFormatex.wFormatTag = WAVE_FORMAT_PCM;
    }

    bool init(AVCodecContext *ctx, const core_audio_sample &sample) override
    {
        initSample(ctx);
//        if(m_ds8)
//        {
//            return true;
//        }

        initDS();
        setWaveInfo(static_cast<DWORD>(sample.rate), static_cast<WORD>(sample.channels), 16);

        //second buffer
        DSBUFFERDESC desc1;
        memset(&desc1, 0x00, sizeof(DSBUFFERDESC));
        desc1.dwSize         = sizeof(DSBUFFERDESC);
        desc1.dwFlags        = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
        desc1.dwBufferBytes  = /*m_waveFormatex.nAvgBytesPerSec * 2*/m_frameSize * m_frameCount;
        desc1.lpwfxFormat    = &m_waveFormatex;
        m_ds8->CreateSoundBuffer(&desc1, &m_dsSecondBuffer, nullptr);

        //gen buffer
        m_dsSecondBuffer->QueryInterface(IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&m_ds8Buffer));
        m_ds8Buffer->QueryInterface(IID_IDirectSoundNotify, reinterpret_cast<void**>(&m_ds8Notify));

        for(unsigned int i = 0; i < m_frameCount; ++i)
        {
            m_ds8PosNotify[i].dwOffset      = i * m_frameSize;
            m_ds8PosNotify[i].hEventNotify  = m_event;
        }

        m_ds8Notify->SetNotificationPositions(m_frameCount, m_ds8PosNotify);
        m_ds8Notify->Release();

        initThread();

        m_ds8Buffer->SetCurrentPosition(0);
        auto code = m_ds8Buffer->Play(0, 0, DSBPLAY_LOOPING);
        Log(Log_Debug, "!!!!!!!play[%p]", code);
        return true;
    }

    void stop() override
    {
        core_devPrivate::stop();
        SAFE_RELEASE(m_ds8Buffer)
        SAFE_RELEASE(m_dsSecondBuffer)
        SetEvent(m_event);
        cleanBuffer();
    }

    bool waitPlay() override
    {
        if(!core_devPrivate::waitPlay())
            return false;

        if(WaitForSingleObject(m_event, 2000) != WAIT_OBJECT_0)
            return false;

        return true;
    }

    void* getLockBuffer(unsigned int len)
    {
        if(!m_ds8Buffer)
            return nullptr;

        DWORD dwPlay = 0;
        DWORD dwWrite = 0;
        auto hr = m_ds8Buffer->GetCurrentPosition(&dwPlay, &dwWrite);
        if(hr == DSERR_BUFFERLOST)
        {
            m_ds8Buffer->Restore();
            hr = m_ds8Buffer->GetCurrentPosition(&dwPlay, &dwWrite);
        }

        if(FAILED(hr))
        {
            return nullptr;
        }

        dwWrite /= m_frameSize;
        dwWrite = (dwWrite + 1) % m_frameCount;
        dwWrite *= m_frameSize;

        void *data      = nullptr;
        DWORD data_len  = 0;
        hr = m_ds8Buffer->Lock(dwWrite, len, &data, &data_len, nullptr, &dwPlay, 0);
        if(hr == DSERR_BUFFERLOST)
        {
            m_ds8Buffer->Restore();
            hr = m_ds8Buffer->Lock(dwWrite, len, &data, &data_len, nullptr, &dwPlay, 0);
        }

        if(FAILED(hr))
        {
            Log(Log_Debug, "lock failed %p[pos=%-7d len=%-7d data_len=%-7d play=%-7d write=%-7d]"
                , hr, m_pos, len, data_len, dwPlay, dwWrite);
            return nullptr;
        }

        if(len != data_len)
        {
            Log(Log_Debug, "dsound diffrent play len[%-7d] data_len[%-7d]", len, data_len);
        }
        return data;
    }

    bool releaseBuffer(void* buffer, unsigned len)
    {
        auto hr = m_ds8Buffer->Unlock(buffer, len, nullptr, 0);
        if(hr != DS_OK)
        {
            //DSERR_INVALIDPARAM
            Log(Log_Debug, "unlock failed %p", hr);
            return false;
        }

        return true;
    }

    void play() override
    {
        bool bZero = false;
        auto len = m_read > m_frameSize ? m_frameSize : m_read;

        if(!m_read)
        {
            Log(Log_Debug, "m_read=%-7d m_write=%-7d m_pos=%-7d len=%-7d", m_read, m_write, m_pos, len);
            bZero = true;
            len = m_frameSize;
        }

        auto data = getLockBuffer(len);
        if(!data)
        {
            return;
        }
        auto data_len = len;

        if(!bZero)
        {
            memcpy(data, m_buffer + m_pos, data_len);
            m_pos += data_len;
            m_pos %= m_frameSize * m_frameCount;
        }
        else
            memset(data, 0x00, data_len);

        if(!releaseBuffer(data, data_len))
            return;

        if(m_read >= data_len && !bZero)
            m_read -= data_len;
    }


    IDirectSound8       *m_ds8;
    IDirectSoundBuffer8 *m_ds8Buffer;
    IDirectSoundBuffer  *m_dsSecondBuffer;
    IDirectSoundNotify8 *m_ds8Notify;
    DSBPOSITIONNOTIFY   *m_ds8PosNotify;
    HANDLE              m_event;
    WAVEFORMATEX        m_waveFormatex;
};

core_dev_dsound::core_dev_dsound()
    : core_dev(new core_dev_dsoundPrivate(this))
{

}
