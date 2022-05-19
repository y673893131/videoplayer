#ifndef CORE_DEVPRIVATE_H
#define CORE_DEVPRIVATE_H

#include <video_pimpl.h>
#include "../convert/core_convert_define.h"
#include "../sample/core_audio_sample.h"
#include "../lock/core_graud_lock.h"
#include "../common.h"
#include <thread>

class core_dev;
class core_devPrivate : public VP_Data<core_dev>
{
    VP_DECLARE_PUBLIC(core_dev)

protected:
    enum audio_state
    {
        audio_uninit,
        audio_init,
        audio_running,
        audio_pause,
        audio_stop,
        audio_reset,
    };
public:
    core_devPrivate(core_dev* parent);
    virtual ~core_devPrivate();

    virtual bool init(AVCodecContext* ctx, const core_audio_sample& sample) = 0;
    virtual unsigned int sample();
    virtual void start();
    virtual void stop();
    virtual void pause();
    virtual void reset();
    virtual void append(uint8_t *dst, uint8_t *src, unsigned int len, int vol);
    virtual bool waitPlay();
    virtual bool waitDecode();
    virtual void play() = 0;

protected:
    void initSample(AVCodecContext* ctx);
    void initThread();
    void decodeEntry();
    void playEntry();
    virtual void cleanBuffer();
    void appendBuffer(uint8_t *src, unsigned int len);
    virtual void getBuffer(uint8_t **dst);
#ifdef WIN32
    void setHighPriority(HANDLE& task, bool bSet);
#endif
protected:
    audioCallback   m_callback;
    void            *m_userData;
    audio_state     m_state;
    unsigned char   *m_buffer;
    unsigned int    m_read;
    unsigned int    m_write;
    unsigned int    m_pos;
    unsigned int    m_samples;
    unsigned int    m_frameCount;
    unsigned int    m_frameSize;
    bool            m_bQuitPlay;
    bool            m_bQuitDecode;
    core_graud_lock m_lock;
};

#endif // CORE_DEVPRIVATE_H
