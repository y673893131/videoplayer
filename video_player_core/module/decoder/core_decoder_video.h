#ifndef CORE_DECODER_VIDEO_H
#define CORE_DECODER_VIDEO_H

#include "core_decoder.h"
#include "../packet/core_packets.h"
#include "../convert/core_frame_convert.h"

class core_decoder_video : public core_decoder
{
public:
    core_decoder_video();

    virtual ~core_decoder_video() override;

    bool init(AVFormatContext* format, int index) override;
    void uninit() override;

    void setSize(int,int);
    int width();
    int height();

    void scale(AVFrame* src, void* cb);
private:
    core_frame_convert* m_convert;
    std::function<void ()> callback;
};

#endif // CORE_DECODER_VIDEO_H
