#include "core_dev_p.h"
#include <thread>
#include "../util/core_util.h"

#ifdef WIN32
#include <avrt.h>
#endif

#define FRAME_SIZE		(4*1024)
#define FRAME_COUNT     4
//#define FRAME_SIZE      192000
//#define FRAME_SIZE      176400/4
//#define FRAME_COUNT     2

core_devPrivate::core_devPrivate(core_dev* parent)
    : VP_Data(parent)
    , m_callback(nullptr)
    , m_userData(nullptr)
    , m_state(audio_uninit)
    , m_buffer(nullptr)
    , m_read(0)
    , m_write(0)
    , m_pos(0)
    , m_samples(0)
    , m_frameCount(FRAME_COUNT)
    , m_frameSize(FRAME_SIZE)
{
    m_buffer = new unsigned char[m_frameCount * m_frameSize];
    cleanBuffer();
}

core_devPrivate::~core_devPrivate()
{
}

unsigned int core_devPrivate::sample()
{
    return m_samples;
}

void core_devPrivate::start()
{
    if(m_state != audio_reset)
        m_state = audio_running;
}

void core_devPrivate::stop()
{
    m_bQuitPlay = false;
    m_bQuitDecode = false;
    m_state = audio_stop;
    while(m_bQuitPlay && m_bQuitDecode) msleep(1);
}

void core_devPrivate::pause()
{
    m_state = audio_pause;
}

void core_devPrivate::reset()
{
    m_state = audio_reset;
}

void core_devPrivate::append(uint8_t *, uint8_t *src, unsigned int len, int)
{
    appendBuffer(src, len);
}

bool core_devPrivate::waitPlay()
{
    if(m_read < m_frameSize)
    {
        Log(Log_Debug, "play continue.");
        msleep(1);
        return false;
    }

    return true;
}

bool core_devPrivate::waitDecode()
{
    if(m_read >= 2 * m_frameSize)
    {
        msleep(10);
        return false;
    }

    return true;
}

void core_devPrivate::initSample(AVCodecContext* ctx)
{
#ifdef AUDIO_FILTER
    m_samples = static_cast<unsigned int>(ctx->frame_size);
    if(!m_samples)
        m_samples = 4096;
#else
    m_samples = 4096;
#endif
}

void core_devPrivate::initThread()
{
    m_state = audio_init;
    std::thread(&core_devPrivate::decodeEntry, this).detach();
    std::thread(&core_devPrivate::playEntry, this).detach();
}

void core_devPrivate::decodeEntry()
{
    Log(Log_Debug, "enter.[%d]", core_util::getThreadId());
#ifdef WIN32
    HANDLE task = nullptr;
    setHighPriority(task, true);
#endif

    cleanBuffer();
    for(;;)
    {
        if(m_state != audio_running)
        {
            if(m_state == audio_reset)
            {
                LOCK(m_lock)
                cleanBuffer();
                while(m_read < m_frameSize)
                {
                    if(m_state == audio_stop)
                        break;

                    m_callback(m_userData, nullptr, static_cast<int>(m_frameSize));
                }

                if(m_state == audio_stop)
                    break;
                m_state = audio_running;
                continue;
            }
            else if(m_state == audio_stop)
            {
                break;
            }
            msleep(1);
            continue;
        }

        if(!waitDecode())
        {
            continue;
        }

        m_callback(m_userData, nullptr, static_cast<int>(m_frameSize));
    }

#ifdef WIN32
    setHighPriority(task, false);
#endif
    Log(Log_Debug, "quit.");
    m_bQuitDecode = true;
}

void core_devPrivate::playEntry()
{
    Log(Log_Debug, "enter.[%d]", core_util::getThreadId());
#ifdef WIN32
    HANDLE task = nullptr;
    setHighPriority(task, true);
#endif
    for(;;)
    {
        if(m_state != audio_running)
        {
            if(m_state == audio_stop)
            {
                break;
            }
            else if(m_state == audio_reset)
            {
                continue;
            }

            msleep(1);
            continue;
        }

        if(!waitPlay())
        {
            continue;
        }

        if(m_state == audio_reset)
        {
            continue;
        }
        {
            LOCK(m_lock)
            if(m_read < m_frameSize)
            {
                continue;
            }
            play();
        }
    }
#ifdef WIN32
    setHighPriority(task, false);
#endif
    Log(Log_Debug, "quit.");
    m_bQuitPlay = true;
}

void core_devPrivate::cleanBuffer()
{
    memset(m_buffer, 0x00, sizeof(unsigned char) * m_frameCount * m_frameSize);
    m_write = 0;
    m_read = 0;
    m_pos = 0;
}

void core_devPrivate::appendBuffer(uint8_t *src, unsigned int len)
{
    if(!len)
        return;
//    Log(Log_Debug, "tid=%d read=%d[%d] m_write=%d[%d]", ::GetCurrentThreadId(), m_read, len, m_write, m_pos);
    unsigned size = m_frameSize * m_frameCount;
    if(m_write + len > size)
    {
        auto block0 = size - m_write;
        memcpy(m_buffer + m_write, src, block0);
        m_write = len - block0;
        memcpy(m_buffer, src + block0, m_write);
    }
    else
    {
        memcpy(m_buffer + m_write, src, len);
        m_write += len;
    }

    m_read += len;
}

void core_devPrivate::getBuffer(uint8_t **dst)
{
    auto read = m_read;
    if(read <= 0)
    {
        memset(*dst, 0x00, m_frameSize);
        return;
    }

    read = read < m_frameSize ? read : m_frameSize;
    if(read < m_frameSize)
    {
        memset(*dst, 0x00, m_frameSize);
        memcpy(*dst, m_buffer + m_pos, read);
    }
    else
    {
        memcpy(*dst, m_buffer + m_pos, read);
    }

//    Log(Log_Debug, "read=%d read=%d[%d] m_write=%d[%d]", ::GetCurrentThreadId(), m_read, read, m_write, m_pos);
    m_read -= read;
    m_pos += read;
    m_pos %= m_frameSize * m_frameCount;
}

#ifdef WIN32
void core_devPrivate::setHighPriority(HANDLE& task, bool bSet)
{
    if(bSet)
    {
        /*
            set Priority
            HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile
            https://docs.microsoft.com/zh-cn/windows/win32/procthread/multimedia-class-scheduler-service?redirectedfrom=MSDN
        */
        DWORD idx = 0;
        task = AvSetMmThreadCharacteristicsW(L"Pro Audio", &idx);
    }
    else if(task)
    {
        AvRevertMmThreadCharacteristics(task);
        task = nullptr;
    }
}
#endif
