#include "core_dev.h"
#include "core_dev_p.h"

core_dev::core_dev(core_devPrivate* pri)
    : VP_INIT(pri)
{
}

core_dev::~core_dev()
{
}

void core_dev::setCallBack(audioCallback callback, void *userdata)
{
    VP_D(core_dev);
    d->m_callback = callback;
    d->m_userData = userdata;
}

unsigned int core_dev::sample()
{
    VP_D(core_dev);
    return d->sample();
}

bool core_dev::init(AVCodecContext *ctx, const core_audio_sample &sample)
{
    VP_D(core_dev);
    d->cleanBuffer();
    return d->init(ctx, sample);
}

void core_dev::start()
{
    VP_D(core_dev);
    d->start();
}

void core_dev::stop()
{
    VP_D(core_dev);
    d->stop();
}

void core_dev::pause()
{
    VP_D(core_dev);
    d->pause();
}

void core_dev::reset()
{
    VP_D(core_dev);
    d->reset();
}

void core_dev::play(uint8_t *dst, uint8_t *src, unsigned int len, int vol)
{
    VP_D(core_dev);
    d->append(dst, src, len, vol);
}
