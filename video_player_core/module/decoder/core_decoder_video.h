#ifndef CORE_DECODER_VIDEO_H
#define CORE_DECODER_VIDEO_H

#include "core_decoder_hardware.h"
#include "../packet/core_packets.h"
#include "../convert/core_frame_convert.h"

class core_decoder_video : public core_decoder_hardware
{
public:
    core_decoder_video();

    virtual ~core_decoder_video() override;

    bool init(AVFormatContext*, int) override;
    void uninit() override;
    bool decode(AVPacket *pk) override;
    bool checkSeekPkt(AVPacket *pk) override;
    bool setDecodeType(int);
    int getDecodeType();

    void setSize(int,int);
    int width();
    int height();

    void displayFrame(video_interface* cb);
private:
    core_frame_convert* m_convert;
    int m_decodeType;
};

#endif // CORE_DECODER_VIDEO_H
