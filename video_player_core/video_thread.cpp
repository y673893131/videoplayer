#include "video_thread.h"
#include "Log/Log.h"
#include <video_player_core.h>
#include <thread>
#include <memory>
#ifdef unix
#include <pthread.h>
#endif
std::vector<video_thread*> video_thread::m_threads;
video_thread::video_thread(const core_media& info)
{
    m_media = new core_media(info);
    m_threads.push_back(this);
    m_index = m_threads.size() - 1;
    Log(Log_Info, "thread begin.%d",std::this_thread::get_id());
}

video_thread::video_thread(const video_thread &)
{
    Log(Log_Info, "thread over.%d",std::this_thread::get_id());
}

video_thread::~video_thread()
{
    Log(Log_Info, "thread exit.%d", std::this_thread::get_id());
    SAFE_RELEASE_PTR(&m_media)
}

void video_thread::start(const core_media& arg)
{
    std::thread([arg]()
    {
        auto vthread = new video_thread(arg);
        vthread->startPlay();
    }).detach();
}

video_thread *video_thread::index(size_t index)
{
    if(m_threads.size() > index)
        return m_threads[index];
//    Log(Log_Warning, "index %d is not exist. size is %d", index, m_threads.size());
    return nullptr;
}

void video_thread::setSize(int w, int h)
{
    m_media->setSize(w, h);
}

void video_thread::play()
{
    m_media->play();
}

void video_thread::setPause()
{
    m_media->setPause();
}

bool video_thread::getPause()
{
    return m_media->isPause();
}

void video_thread::continuePlay()
{
    m_media->continuePlay();
}

void video_thread::setStop()
{
    m_media->stop();
}

bool video_thread::getStop()
{
    return m_media->isStop();
}

void video_thread::seekPos(int64_t ms)
{
    m_media->seek(ms);
}

bool video_thread::seekJump(int64_t ms)
{
    return m_media->jump(ms);
}

void video_thread::getSeekImg(int64_t ms)
{
    m_media->preview(ms);
}

void video_thread::setVol(int nVol)
{
    m_media->setVol(nVol);
//    m_media->audio->sdl->nVol = nVol;
}

void video_thread::setMute(bool bMute)
{
    m_media->mute(bMute);
}

void video_thread::setAudioChannel(audio_channel_type type)
{
    m_media->setAudioChannel(type);
}

void video_thread::setChannel(int channel, int sel)
{
    m_media->setChannel(channel, sel);
}

void video_thread::setDecode(int type)
{
    m_media->setDecode(type);
}

void video_thread::setSpeed(int type)
{
    m_media->setSpeed(type);
}

int video_thread::state()
{
    return m_media->state();
}

video_player_core::enum_state video_thread::state1()
{
    return m_media->state1();
}

int video_thread::setCapture(bool bCap)
{
    return m_media->setCapture(bCap);
}

void video_thread::startPlay()
{
    m_media->open(m_index);

    auto _cb = reinterpret_cast<video_interface*>(m_media->_cb);
    if(_cb)
    {
        auto index = m_index;
        SAFE_RELEASE_PTR(&m_threads[index]);
        _cb->endCall(index);
        OUT_PUT_LEAK();
    }
}
