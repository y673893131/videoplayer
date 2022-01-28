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
    bool start(core_media* media, int nIndex);
    void stop();

protected:
    virtual bool init(core_media* media, int nIndex);
    virtual void uninit();
    virtual std::string outoutFile() = 0;
    AVOutputFormat *guess(core_media* media);
    bool initStream(core_media* media,int nIndex);
    bool initHeader();
    void rescalePacket(AVPacket* pk);
protected:
    std::string m_sFileName;
    AVFormatContext* m_format;
    AVStream* m_stream;
    AVStream* m_in;
    int64_t m_lastDts;
};

#endif // CORE_SAVE_BASE_H
