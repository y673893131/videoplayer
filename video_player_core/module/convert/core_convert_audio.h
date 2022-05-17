#ifndef CORE_CONVERT_AUDIO_H
#define CORE_CONVERT_AUDIO_H

#include "core_convert.h"
#include "core_convert_define.h"
#include "video_player_core.h"

class core_dev;
class core_convert_audioPrivate;
class core_convert_audio : public core_convert
{
    VP_DECLARE_PRIVATE(core_convert_audio)
public:
    core_convert_audio();
    ~core_convert_audio() override;

    void setContext(AVCodecContext *ctx) override;

    void formatChannelType(Uint8* buff, unsigned int size);
    bool formatFreq(Uint8 *buff, unsigned int size, video_interface *cb);
};

#endif // CORE_CONVERT_AUDIO_H
