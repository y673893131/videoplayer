#ifndef CORE_DEV_H
#define CORE_DEV_H

#include <video_pimpl.h>
#include "../convert/core_convert_define.h"
#include "../sample/core_audio_sample.h"
#include "../common.h"

class core_devPrivate;
class core_dev
{
protected:
    VP_DECLARE(core_dev)
    VP_DECLARE_PRIVATE(core_dev)
public:
    core_dev(core_devPrivate*);
    virtual ~core_dev();

    void setCallBack(audioCallback callback, void *userdata);

    virtual unsigned int sample();
    virtual bool init(AVCodecContext* ctx, const core_audio_sample& sample);
    virtual void start();
    virtual void stop();
    virtual void pause();
    virtual void reset();
    virtual void play(uint8_t *dst, uint8_t *src, unsigned int len, int vol);
};

#endif // CORE_DEV_H
