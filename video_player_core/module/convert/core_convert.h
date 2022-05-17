#ifndef CORE_CONVERT_H
#define CORE_CONVERT_H

#include "../common.h"
#include <video_pimpl.h>

class core_dev;
class core_convertPrivate;
class core_convert
{
protected:
    VP_DECLARE(core_convert)
    VP_DECLARE_PRIVATE(core_convert)
public:
    core_convert(core_convertPrivate*);
    virtual ~core_convert();

    virtual void setContext(AVCodecContext* ctx);
    AVCodecContext *getContext();

    virtual bool convert(AVFrame *src);
    unsigned char *buffer();
    unsigned int size();

    void reset();
    bool initResample();
    void setVolume(int);
    int getVolume();
    void setChannelType(int type);
    void start();
    void pause();
    void stop();
    core_dev *dev();
};

#endif // CORE_CONVERT_H
