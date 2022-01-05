#ifndef CORE_DECODER_HARDWARE_H
#define CORE_DECODER_HARDWARE_H

#include "core_decoder.h"
#include <map>

class core_decoder_hardware : public core_decoder
{
public:
    core_decoder_hardware();
    virtual ~core_decoder_hardware() override;

    std::map<int, std::string> getSupportDevices();

    bool init(AVFormatContext *, int) override;
    bool initOtherHw(AVFormatContext*, int);
    bool initQsv(AVFormatContext*, int);

    void uninit() override;
    bool decode(AVPacket* pk, bool& bTryAgain) override;
    virtual bool checkSeekPkt(AVPacket *pk);
    bool isIFrame(AVPacket *pk);
    void checkQSVClock(AVPacket* pk, int64_t& pts) override;
private:
    static AVPixelFormat getFormatOtherHw(AVCodecContext* ctx, const AVPixelFormat* srcFormat);
    static AVPixelFormat getFormatQSV(AVCodecContext* ctx, const AVPixelFormat* srcFormat);
    int initQSV(AVCodecContext* ctx);
protected:
    bool m_bFirstSeek;
    int m_decodeType;

private:
    AVHWDeviceType m_devType;
    AVBufferRef* m_buffer;
    AVFrame* m_hwframe;
    AVPixelFormat m_pixFormat;
};

#endif // CORE_DECODER_HARDWARE_H
