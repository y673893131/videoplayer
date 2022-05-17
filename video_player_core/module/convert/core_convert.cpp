#include "core_convert.h"
#include "core_convert_p.h"
#include "../dev/core_dev.h"

core_convert::core_convert(core_convertPrivate* pri)
    : VP_INIT(pri)
{
}

core_convert::~core_convert()
{
}

void core_convert::setContext(AVCodecContext *ctx)
{
    VP_D(core_convert);
    d->m_codecCtx = ctx;
}

AVCodecContext *core_convert::getContext()
{
    VP_D(core_convert);
    return d->m_codecCtx;
}

unsigned char *core_convert::buffer()
{
    VP_D(core_convert);
    return d->m_buffer;
}

unsigned int core_convert::size()
{
    VP_D(core_convert);
    return d->m_size;
}

bool core_convert::convert(AVFrame *src)
{
    VP_D(core_convert);
    return d->convert(src);
}

void core_convert::reset()
{
    VP_D(core_convert);
    d->reset();
    auto dev = d->dev();
    if(dev)
        dev->reset();
}

bool core_convert::initResample()
{
    VP_D(core_convert);
    return d->initResample();
}

void core_convert::setVolume(int vol)
{
    VP_D(core_convert);
    d->setVolume(vol);
}

int core_convert::getVolume()
{
    VP_D(core_convert);
    return d->getVolume();
}

void core_convert::setChannelType(int type)
{
    VP_D(core_convert);
    d->setChannelType(type);
}

void core_convert::start()
{
    VP_D(core_convert);
    auto dev = d->dev();
    if(dev)
        dev->start();
}

void core_convert::pause()
{
    VP_D(core_convert);
    auto dev = d->dev();
    if(dev)
        dev->pause();
}

void core_convert::stop()
{
    VP_D(core_convert);
    auto dev = d->dev();
    if(dev)
        dev->stop();
}

core_dev *core_convert::dev()
{
    VP_D(core_convert);
    return d->dev();
}
