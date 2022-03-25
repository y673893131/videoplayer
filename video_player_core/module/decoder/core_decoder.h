#ifndef CORE_DECODER_H
#define CORE_DECODER_H

#include "../common.h"
#include <set>
#include "../packet/core_packets.h"

class core_thread_video;
class core_thread_audio;
class core_thread_subtitle;
class core_media;
class core_save;
class core_save_base;
class core_filter_base;
class core_decoder
{
public:
    core_decoder();
    virtual ~core_decoder();

    virtual bool init(AVFormatContext*, int);
    virtual void uninit();
    virtual bool decode(AVPacket* pk, bool& bTryAgain) = 0;
    virtual bool checkSeekPkt(AVPacket *pk);
    bool check(int);
    bool isValid();
    void pushStream(unsigned int);
    int& index();
    virtual void setIndex(int);
    std::set<unsigned int> &indexs();

    unsigned int pktSize();
    core_packets& pkts();
    void flush();

    int64_t clock();
    int64_t displayClock();
    void setClock(int64_t clock);
    void calcClock(AVPacket* pk);
    virtual void checkQSVClock(AVPacket* pk, int64_t& pts);
    int64_t getInteralPts(int64_t pos);
    int64_t getDisplayPts(int64_t pos);
protected:
    int nStreamIndex;
    std::set<unsigned int> nStreamIndexs;
    AVFormatContext *format;
    AVFrame* frame;
    AVCodecContext* pCodecContext;
    AVCodec* pCodec;
    AVStream* stream;
    core_packets pks;
    int64_t pts;
    core_filter_base* m_filter;

    friend class core_thread_video;
    friend class core_thread_demux;
    friend class core_thread_audio;
    friend class core_thread_subtitle;
    friend class core_media;
    friend class core_save;
    friend class core_save_base;
};

#endif // CORE_DECODER_H
