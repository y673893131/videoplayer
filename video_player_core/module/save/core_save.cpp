#include "core_save.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"
#include "core_save_audio.h"
#include "core_save_video.h"

core_save::core_save()
    :m_audio(nullptr)
    ,m_video(nullptr)
    ,m_inputFormat(nullptr)
    ,m_nAudioIndex(-1)
    ,m_nVideoIndex(-1)
{
}

core_save::~core_save()
{

}

bool core_save::init(AVFormatContext* pFormat, int nIndexVideo, int nIndexAudio)
{
    uninit();

    m_inputFormat = pFormat;
    m_nAudioIndex = nIndexAudio;
    m_nVideoIndex = nIndexVideo;

    INIT_NEW(&m_audio, core_save_audio)
    INIT_NEW(&m_video, core_save_video)

    return true;
}

void core_save::uninit()
{
    SAFE_RELEASE_PTR(&m_audio)
    SAFE_RELEASE_PTR(&m_video)
}

void core_save::start()
{
    m_audio->start(m_inputFormat, m_nAudioIndex);
    m_video->start(m_inputFormat, m_nVideoIndex);
}

void core_save::stop()
{
    m_audio->stop();
    m_video->stop();
}

void core_save::saveAudio(AVPacket* pk)
{
    m_audio->save(pk);
}

void core_save::saveVideo(AVPacket* pk)
{
    m_video->save(pk);
}
