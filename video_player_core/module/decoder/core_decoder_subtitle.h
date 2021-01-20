#ifndef CORE_DECODER_SUBTITLE_H
#define CORE_DECODER_SUBTITLE_H

#include "core_decoder.h"

class core_decoder_subtitle : public core_decoder
{
public:
    core_decoder_subtitle();
    virtual ~core_decoder_subtitle() override;

    bool init(AVFormatContext* format, int index) override;
    void uninit() override;

    AVSubtitle* subtitle();
private:
    AVSubtitle* pSubtitle;
};

#endif // CORE_DECODER_SUBTITLE_H
