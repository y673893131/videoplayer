#ifndef CORE_FILTER_H
#define CORE_FILTER_H

#include "video_player_core.h"
#include "../common.h"

enum core_filter_type
{
    filter_volume,
};

class core_filter_base;
class core_filter
{
public:
    core_filter();
    virtual ~core_filter();

public:
    template<class T>
    bool init(AVStream* pStream, AVCodecContext* pCodecContext);
    void uninit();

    void setValue(core_filter_type type, int);

    AVFrame* mix(AVFrame* in);
private:
    core_filter_base* m_filter;
    core_filter_base* m_filterTmp;

    AVStream* m_pStream;
    AVCodecContext* m_pCodecContext;
};

#endif // CORE_FILTER_H
