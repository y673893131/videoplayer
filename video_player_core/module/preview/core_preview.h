#ifndef CORE_PREVIEW_H
#define CORE_PREVIEW_H

#include "../decoder/core_decoder_video.h"
#include "../decoder/core_decoder_subtitle.h"

class core_preview
{
public:
    core_preview();
    virtual ~core_preview();

    bool isOk();
    void init(const std::string& file);
    void preview(const std::string& src, int64_t ms, void* cb);
    void clear();

private:
    std::string sFile;
    int nIndex;
    AVFrame* frame;
    AVFormatContext* pFormatCtx;
    AVCodecContext* ctx;
    core_frame_convert* outFrame;
};

#endif // CORE_PREVIEW_H
