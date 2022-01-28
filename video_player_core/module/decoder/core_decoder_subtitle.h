#ifndef CORE_DECODER_SUBTITLE_H
#define CORE_DECODER_SUBTITLE_H

#include "core_decoder.h"
#include "video_player_core.h"

class core_decoder_subtitle : public core_decoder
{
public:
    core_decoder_subtitle();
    virtual ~core_decoder_subtitle() override;

    bool init(AVFormatContext* format, int index) override;
    void uninit() override;

    bool decode(AVPacket* pk, bool& bTryAgain) override;
    AVSubtitle* subtitle();
    void subtitleHeader(video_interface *cb);
    void displayFrame(video_interface *cb, int64_t startTime);

private:
    void analyzeAASHeader(subtitle_header& header, const std::string& desc);
private:
    AVSubtitle* pSubtitle;
    int64_t _ptsEnd;
};

#endif // CORE_DECODER_SUBTITLE_H
