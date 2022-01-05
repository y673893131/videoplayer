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
    bool init(AVFormatContext* pFormat, int nIndex) override;
    std::string outoutFile() override;
    AVOutputFormat *guess() override;
    void uninit() override;

    bool initBsf();
private:
    AVBSFContext *m_bsfCtx;
};

#endif // CORE_SAVE_VIDEO_H
