#ifndef CORE_SAVE_BASE_H
#define CORE_SAVE_BASE_H

#include "video_player_core.h"
#include "../common.h"

class core_save_base
{
public:
    core_save_base();
    virtual ~core_save_base();

public:
    virtual void save(AVPacket*) = 0;
    bool start(AVFormatContext* pFormat, int nIndex);
    void stop();

protected:
    virtual bool init(AVFormatContext* pFormat, int nIndex);
    virtual void uninit();
    virtual std::string outoutFile() = 0;
    virtual AVOutputFormat* guess() = 0;
    bool initStream(AVFormatContext* pFormat, int nIndex);
    bool initHeader();
    void rescalePacket(AVPacket* pk);
protected:
    std::string m_sFileName;
    AVFormatContext* m_format;
    AVStream* m_stream;
    AVStream* m_in;
    _int64 m_lastDts;
};

#endif // CORE_SAVE_BASE_H
