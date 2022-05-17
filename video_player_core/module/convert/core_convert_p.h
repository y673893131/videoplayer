#ifndef CORE_CONVERTPRIVATE_H
#define CORE_CONVERTPRIVATE_H

#include <video_pimpl.h>
#include "../common.h"

class core_dev;
class core_convert;
class core_convertPrivate : public VP_Data<core_convert>
{
    VP_DECLARE_PUBLIC(core_convert)
public:
    core_convertPrivate(core_convert*);

    virtual void initFrame();
    void unInitFrame();

    virtual bool initResample() = 0;
    virtual bool convert(AVFrame*) = 0;
    virtual core_dev *dev() = 0;
    virtual void reset() = 0;
    virtual void setVolume(int);
    virtual int  getVolume();
    virtual void setChannelType(int type);
protected:
    AVFrame         *m_frame;
    uint8_t         *m_buffer;
    unsigned int    m_size;
    AVCodecContext  *m_codecCtx;
};

#endif // CORE_CONVERTPRIVATE_H
