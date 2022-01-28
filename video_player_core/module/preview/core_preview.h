#ifndef CORE_PREVIEW_H
#define CORE_PREVIEW_H

#include "../decoder/core_decoder_video.h"
#include "../decoder/core_decoder_subtitle.h"

class core_preview
{
public:
    core_preview();
    virtual ~core_preview();

    void setCallBack(video_interface* cb);
    void preview(const std::string& src, int64_t ms, int flag);

private:
    bool isOk();
    void init(const std::string& file);
    void clear();
    void threadCall();
private:
    int flag;
    std::string src;
    int64_t ms;
    int64_t dstms;
    bool bFlag;
    bool bEnd;

    video_interface* cb;
    int64_t start_time;
    std::string sFile;
    int nIndex;
    AVStream* stream;
    AVFrame* frame;
    AVFormatContext* pFormatCtx;
    AVCodecContext* ctx;
    core_frame_convert* outFrame;
};

#endif // CORE_PREVIEW_H
