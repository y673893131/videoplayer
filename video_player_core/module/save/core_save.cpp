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
    ,m_media(nullptr)
    ,m_nAudioIndex(-1)
    ,m_nVideoIndex(-1)
{
}

core_save::~core_save()
{
    uninit();
}

bool core_save::init(core_media* media, int nIndexVideo, int nIndexAudio)
{
    uninit();

    m_media = media;
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
    if(m_nAudioIndex >= 0) m_audio->start(m_media, m_nAudioIndex);
    if(m_nVideoIndex >= 0) m_video->start(m_media, m_nVideoIndex);
}

void core_save::stop()
{
    if(m_nAudioIndex >= 0) m_audio->stop();
    if(m_nVideoIndex >= 0) m_video->stop();
}

void core_save::saveAudio(AVPacket* pk)
{
    if(m_nAudioIndex >= 0) m_audio->save(pk);
}

void core_save::saveVideo(AVPacket* pk)
{
    if(m_nVideoIndex >= 0) m_video->save(pk);
}
