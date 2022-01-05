#ifndef CORE_SAVE_H
#define CORE_SAVE_H

#include "video_player_core.h"
#include "../common.h"

class core_save_audio;
class core_save_video;
class core_save
{
public:
    core_save();
    virtual ~core_save();

public:
    bool init(AVFormatContext* pFormat, int nIndexVideo, int nIndexAudio);
    void uninit();

    void start();
    void stop();

    void saveAudio(AVPacket*);
    void saveVideo(AVPacket*);

private:

    core_save_audio* m_audio;
    core_save_video* m_video;

    AVFormatContext* m_inputFormat;
    int m_nAudioIndex;
    int m_nVideoIndex;
};

#endif // CORE_SAVE_H
