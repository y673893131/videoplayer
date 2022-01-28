#ifndef CORE_FILTER_BASE_H
#define CORE_FILTER_BASE_H

#include "video_player_core.h"
#include "../common.h"

class core_filter_private;
class core_filter_base
{
public:
    core_filter_base();
    virtual ~core_filter_base();

public:
    bool init(AVStream* stream, AVCodecContext* pCodecContext);
    virtual void uninit();
    virtual AVFrame* mix(AVFrame* in);

protected:
    virtual bool initParam(AVStream* stream, AVCodecContext* pCodecContext) = 0;
    virtual bool initFilter() = 0;
    virtual void update() = 0;

    void setFlag(int);
    bool testFlag(int);
protected:
    core_filter_private* m_private;
    unsigned int m_flag;
};

#endif // CORE_FILTER_BASE_H
