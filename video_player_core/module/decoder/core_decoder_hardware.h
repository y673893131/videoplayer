#ifndef CORE_DECODER_HARDWARE_H
#define CORE_DECODER_HARDWARE_H

#include "core_decoder.h"

class core_decoder_hardware : public core_decoder
{
public:
    core_decoder_hardware();
    virtual ~core_decoder_hardware() override;
    bool initCuda(AVFormatContext*, int);
    bool initQsv(AVFormatContext*, int);

    void uninit() override;
    bool decode(AVPacket* pk) override;
    virtual bool checkSeekPkt(AVPacket *pk);
protected:
    void calcClock();
private:
    static AVPixelFormat getFormatCUDA(AVCodecContext* ctx, const AVPixelFormat* srcFormat);
    static AVPixelFormat getFormatQSV(AVCodecContext* ctx, const AVPixelFormat* srcFormat);
    int initQSV(AVCodecContext* ctx);
protected:
    bool m_bFirstSeek;

private:
    AVHWDeviceType m_devType;
    AVBufferRef* m_buffer;
    AVFrame* m_hwframe;
    AVPixelFormat m_pixFormat;
};

#endif // CORE_DECODER_HARDWARE_H
