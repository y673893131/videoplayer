#ifdef WIN32
#include "core_dev_waveout.h"
#include "core_dev_p.h"
#include "../lock/core_graud_lock.h"

#include <list>
#include <mmsystem.h>
#include "MMNotificationClient.h"

class core_dev_waveoutPrivate : public core_devPrivate
{
    VP_DECLARE_PUBLIC(core_dev_waveout)

    inline core_dev_waveoutPrivate(core_dev_waveout* parent)
        : core_devPrivate(parent)
        , m_threadMonitor(nullptr)
        , m_sem(nullptr)
        , m_semMonitor(nullptr)
        , m_hWaveOut(nullptr)
        , m_pda(nullptr)
    {
        initbuffer();
        m_header = new WAVEHDR[m_frameCount];
        memset(m_header, 0x00, sizeof(WAVEHDR) * m_frameCount);

        for (unsigned n = 0; n < m_frameCount; ++n)
        {
            m_header[n].lpData = reinterpret_cast<HPSTR>(m_pda + n * m_frameSize);
            m_header[n].dwBufferLength = m_frameSize;
        }

        m_notify = new CMMNotificationClient(parent);
    }

    void initbuffer()
    {
        uninitbuffer();
        m_pda = new char[m_frameCount * m_frameSize];
        memset(m_pda, 0x00, sizeof(char) * m_frameCount * m_frameSize);
    }

    void uninitbuffer()
    {
        if(m_pda)
        {
            delete[] m_pda;
            m_pda = nullptr;
        }
    }

    static void WINAPI win_callback(void* hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

    static unsigned WINAPI monitor_entry(void *p)
    {
        auto pThis = reinterpret_cast<core_dev_waveoutPrivate*>(p);
        for(;;)
        {
            WaitForSingleObject(pThis->m_semMonitor, INFINITE);

            if(pThis->m_hWaveOut)
            {
                pThis->resetWaveout();
            }
            pThis->m_headers.clear();
            pThis->initDev(core_audio_sample());
        }
    }

    void resetWaveout()
    {
        auto ret = waveOutReset(m_hWaveOut);
        if(ret != MMSYSERR_NOERROR)
        {
            char msg[1024] = {};
            waveOutGetErrorTextA(ret, msg, 1024);
            Log(Log_Err, "waveOutReset failed![%d]%s", ret, msg);
        }
        for (unsigned int n = 0; n < m_frameCount; ++n)
        {
            waveOutUnprepareHeader(m_hWaveOut, &m_header[n], sizeof(WAVEHDR));
        }
    }

    void defaultChanged()
    {
        ReleaseSemaphore(m_semMonitor, 1, nullptr);
    }

    bool initDev(const core_audio_sample& sample)
    {
        if(!m_hWaveOut)
        {
            setWaveInfo(static_cast<DWORD>(sample.rate), static_cast<WORD>(sample.channels), 16);
        }

        auto ret = waveOutOpen(nullptr, WAVE_MAPPER, &m_waveFormatex, 0, 0, WAVE_FORMAT_QUERY);
        if (ret != MMSYSERR_NOERROR)
        {
            char msg[1024] = {};
            waveOutGetErrorTextA(ret, msg, 1024);
            Log(Log_Err, "init wave dev failed!%s", msg);
            return false;
        }

        if(!m_hWaveOut)
        {
            ret = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_waveFormatex, reinterpret_cast<DWORD_PTR>(win_callback), reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
            if (ret != MMSYSERR_NOERROR)
            {
                char msg[1024] = {};
                waveOutGetErrorTextA(ret, msg, 1024);
                Log(Log_Err, "open wave dev failed!%s", msg);
                return false;
            }
        }

        for (unsigned n = 0; n < m_frameCount; ++n)
        {
            ret = waveOutPrepareHeader(m_hWaveOut, &m_header[n], sizeof(WAVEHDR));
            if (ret)
            {
                char buff[1024] = {};
                waveInGetErrorTextA(ret, buff, 1024);
                Log(Log_Err, "waveOutPrepareHeader%d init!%d error:%s", n, ret, buff);
                return false;
            }
        }

        for (unsigned n = 0; n < m_frameCount; ++n)
        {
            waveOutWrite(m_hWaveOut, &m_header[n], sizeof(WAVEHDR));
        }

        return true;
    }

    void stop() override
    {
        core_devPrivate::stop();
        resetWaveout();
    }

    void start() override
    {
        core_devPrivate::start();
        waveOutRestart(m_hWaveOut);
        Log(Log_Debug, "");
    }

    void pause() override
    {
        if(m_state == audio_pause)
            return;
        core_devPrivate::pause();
        waveOutPause(m_hWaveOut);
    }

    void cleanBuffer() override
    {
        core_devPrivate::cleanBuffer();
        ReleaseSemaphore(m_semMonitor, 1, nullptr);
    }

    bool init(AVCodecContext* ctx, const core_audio_sample& sample) override
    {
        Log(Log_Debug, "");
        initSample(ctx);

        if(!initDev(sample))
            return false;

        m_headers.clear();
        if(!m_sem)
        {
            m_sem = CreateSemaphore(nullptr, m_frameCount - 1, m_frameCount, nullptr);
        }

        initThread();

        if(!m_semMonitor)
        {
            m_semMonitor = CreateSemaphore(nullptr, 0, 1, nullptr);
        }

        if(!m_threadMonitor)
        {
            m_threadMonitor = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, monitor_entry, this, 0, nullptr));
        }

        return true;
    }

    void setWaveInfo(DWORD sampleRate, WORD channels, WORD Bits)
    {
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
        Log(Log_Debug, "m_waveFormatex.nAvgBytesPerSec=%d", m_waveFormatex.nAvgBytesPerSec);
    }

    void play() override
    {
        if(m_headers.empty() /*|| !m_hWaveOut || m_state != audio_running*/)
        {
            return;
        }
        auto pHeader = m_headers.front();
        m_headers.pop_front();
        getBuffer(reinterpret_cast<uint8_t**>(&pHeader->lpData));
        for(;;)
        {
            auto ret = waveOutWrite(m_hWaveOut, pHeader, sizeof(WAVEHDR));
            if(ret)
            {
                if(ret == WAVERR_UNPREPARED)
                {
                    msleep(1);
                    continue;
                }
//                else
//                {
//                    char buff[1024] = {};
//                    waveInGetErrorTextA(ret, buff, 1024);
//                    Log(Log_Err, "waveOutWrite %d error:%s", ret, buff);
//                }
            }

            break;
        }

    }

    bool waitPlay() override
    {
        if(!core_devPrivate::waitPlay())
            return false;

        if(WaitForSingleObject(m_sem, 2000) != WAIT_OBJECT_0)
            return false;
        return true;
    }

    HANDLE                  m_threadMonitor;
    HANDLE                  m_sem;
    HANDLE                  m_semMonitor;

    WAVEFORMATEX            m_waveFormatex;
    HWAVEOUT                m_hWaveOut;
    WAVEHDR                 *m_header;
    std::list<LPWAVEHDR>    m_headers;
    char                    *m_pda;

    CMMNotificationClient   *m_notify;
};

void WINAPI core_dev_waveoutPrivate::win_callback(void*, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD)
{
    auto pData = reinterpret_cast<core_dev_waveoutPrivate*>(dwInstance);
    switch (uMsg)
    {
    case WOM_OPEN:
        Log(Log_Info, "open waveout.");
        break;
    case WOM_CLOSE://设备关闭
//        pData->stop();
        Log(Log_Info, "close waveout.");
        break;
    case WOM_DONE://一块数据播放完毕
        pData->m_headers.push_back(reinterpret_cast<LPWAVEHDR>(dwParam1));
        ReleaseSemaphore(pData->m_sem, 1, nullptr);
        break;
    default:
        Log(Log_Debug, "%d", uMsg);
        break;
    }
}

core_dev_waveout::core_dev_waveout()
    : core_dev(new core_dev_waveoutPrivate(this))
{

}

void core_dev_waveout::defaultChanged()
{
    VP_D(core_dev_waveout);
    d->defaultChanged();
}

#endif
