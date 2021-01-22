#ifndef CORE_DECODER_H
#define CORE_DECODER_H

#include "../common.h"
#include <set>
#include "../packet/core_packets.h"

class core_thread_video;
class core_media;
class core_decoder
{
public:
    core_decoder();
    virtual ~core_decoder();

    virtual bool init(AVFormatContext*, int);
    virtual void uninit();
    virtual bool decode(AVPacket* pk);

    bool check(int);
    bool isValid();
    void pushStream(int);
    int& index();
    virtual void setIndex(int);
    std::set<int> &indexs();

    unsigned int pktSize();
    core_packets& pkts();
    void cleanPkt();

    double clock();
    void setClock(double clock);
protected:
    int nStreamIndex;
    std::set<int> nStreamIndexs;
    AVFormatContext *format;
    AVFrame* frame;
    AVCodecContext* pCodecContext;
    AVCodec* pCodec;
    AVStream* stream;
    core_packets pks;
    double _clock;

    friend class core_thread_video;
    friend class core_media;
};

#endif // CORE_DECODER_H
