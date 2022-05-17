#ifndef CORE_DECODER_VIDEO_H
#define CORE_DECODER_VIDEO_H

#include "core_decoder_hardware.h"
#include "../packet/core_packets.h"
#include "../filter/core_filter.h"

class core_decoder_video : public core_decoder_hardware
{
public:
    core_decoder_video();

    virtual ~core_decoder_video() override;

    bool init(AVFormatContext*, int) override;
    void uninit() override;
    bool decode(AVPacket *pk, bool& bTryAgain) override;
    bool checkSeekPkt(AVPacket *pk) override;
    void initDecodeType(int);
    bool setDecodeType(int);
    int getDecodeType();

    int width();
    int height();

    void displayFrame(video_interface* cb);
    bool changeDecodeType(AVPacket *pk,int);
};

#endif // CORE_DECODER_VIDEO_H
