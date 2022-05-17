#include "core_thread_demux.h"
#include "Log/Log.h"
#include <video_player_core.h>
#include "module/filter/core_filter_video.h"
#include <thread>
#include <memory>
#ifdef unix
#include <pthread.h>
#endif
std::vector<core_thread_demux*> core_thread_demux::m_threads;
core_thread_demux::core_thread_demux(const core_media& info)
{
    m_media = new core_media(info);
    m_threads.push_back(this);
    m_index = m_threads.size() - 1;
    Log(Log_Info, "thread begin.%d",std::this_thread::get_id());
}

core_thread_demux::core_thread_demux(const core_thread_demux &)
{
    Log(Log_Info, "thread over.%d",std::this_thread::get_id());
}

core_thread_demux::~core_thread_demux()
{
    Log(Log_Info, "thread exit.%d", std::this_thread::get_id());
    SAFE_RELEASE_PTR(&m_media)
}

unsigned __stdcall core_thread_demux::dumux_entry(void *p)
{
    auto thread = reinterpret_cast<core_thread_demux*>(p);
    thread->startPlay();
    return 0;
}

void core_thread_demux::start(const core_media& arg)
{
#ifdef _WIN32
    auto vthread = new core_thread_demux(arg);
    auto thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, dumux_entry, vthread, 0, nullptr));
    ::SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
    auto pro = ::SetThreadIdealProcessor(thread, MAXIMUM_PROCESSORS);
    ::SetThreadIdealProcessor(thread, pro);
#else
    std::thread([arg]()
    {
        auto thread = GetCurrentThread();
        auto curPri = ::GetThreadPriority(thread);

        Log(Log_Debug, "priority %d -> %d", curPri, 9);
        auto vthread = new core_thread_demux(arg);
        vthread->startPlay();
    }).detach();
#endif
}

core_thread_demux *core_thread_demux::index(size_t index)
{
    if(m_threads.size() > index)
        return m_threads[index];
//    Log(Log_Warning, "index %d is not exist. size is %d", index, m_threads.size());
    return nullptr;
}

void core_thread_demux::play()
{
    m_media->play();
}

void core_thread_demux::setPause()
{
    m_media->setPause();
}

bool core_thread_demux::getPause()
{
    return m_media->isPause();
}

void core_thread_demux::continuePlay()
{
    m_media->continuePlay();
}

void core_thread_demux::setStop()
{
    m_media->stop();
}

bool core_thread_demux::getStop()
{
    return m_media->isStop();
}

void core_thread_demux::seekPos(int64_t ms)
{
    m_media->seek(ms);
}

bool core_thread_demux::seekJump(int64_t ms)
{
    return m_media->jump(ms);
}

void core_thread_demux::getSeekImg(int64_t ms)
{
    m_media->preview(ms);
}

void core_thread_demux::setVol(int nVol)
{
    m_media->setVol(nVol);
//    m_media->audio->sdl->nVol = nVol;
}

void core_thread_demux::setMute(bool bMute)
{
    m_media->mute(bMute);
}

void core_thread_demux::setAudioChannel(audio_channel_type type)
{
    m_media->setAudioChannel(type);
}

void core_thread_demux::setChannel(int channel, int sel)
{
    m_media->setChannel(channel, sel);
}

void core_thread_demux::setDecode(int type)
{
    m_media->setDecode(type);
}

void core_thread_demux::setSpeed(int type)
{
    m_media->setSpeed(type);
}

int core_thread_demux::state()
{
    return m_media->state();
}

video_player_core::enum_state core_thread_demux::state1()
{
    return m_media->state1();
}

int core_thread_demux::setCapture(bool bCap)
{
    return m_media->setCapture(bCap);
}

void *core_thread_demux::frame()
{
    auto filter = reinterpret_cast<core_filter_video*>(m_media->_video->m_filter);
    return filter->buffer();
}

void core_thread_demux::startPlay()
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
