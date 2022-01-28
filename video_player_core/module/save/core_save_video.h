#ifndef CORE_SAVE_VIDEO_H
#define CORE_SAVE_VIDEO_H

#include "video_player_core.h"
#include "../common.h"
#include "core_save_base.h"

class core_save_video : public core_save_base
{
public:
    core_save_video();
    ~core_save_video() override;

public:
    void save(AVPacket*) override;
private:
    bool init(core_media* media, int nIndex) override;
    std::string outoutFile() override;
    void uninit() override;
};

#endif // CORE_SAVE_VIDEO_H
