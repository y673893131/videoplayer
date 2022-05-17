#ifndef CORE_DECODER_AUDIO_H
#define CORE_DECODER_AUDIO_H

#include "core_decoder.h"
//#include "../sdl/core_sdl_op.h"

class core_dev;
class core_decoder_audio : public core_decoder
{
public:
    core_decoder_audio();
    virtual ~core_decoder_audio() override;

    bool init(AVFormatContext* format, int index) override;
    void setIndex(int) override;
    void setVol(int vol);
    int getVol();
    void setAudioChannel(int type);

//    core_sdl_op* sdl();
    core_dev *dev();

    void start();
    void pause();
    bool decode(AVPacket* pk, bool& bTryAgain) override;
    bool change(unsigned int& bufferSize);

private:
//    static core_sdl_op* s_sdl;

    // change channel temp
    int nTempIndex;
};

#endif // CORE_DECODER_AUDIO_H
