#include "core_media.h"
#include "../util/core_util.h"

core_media::core_media()
    :_cb(nullptr)
    ,_state(0)
    ,_start_time(0)
    ,_seek_time(0)
    ,_pause_time(0)
    ,_seek_pos(0)
    ,_flag(0)
    ,_format_ctx(nullptr)
    ,video(nullptr)
    ,audio(nullptr)
    ,_vRead(0)
    ,_aRead(0)
    ,_channel(0)
    ,_channelSel(0)
    ,_decodeType(video_player_core::decode_software)
    ,m_preview(nullptr)
    ,_video_thread(nullptr)
{
    setFlag(flag_bit_Stop);
    _audio_thread = core_thread_audio::instance();
}

core_media::core_media(const core_media &src)
    :_cb(src._cb)
    ,_state(src._state)
    ,_src(src._src)
    ,_start_time(src._seek_time)
    ,_seek_time(static_cast<double>(src._seek_pos))
    ,_pause_time(src._pause_time)
    ,_seek_pos(static_cast<int64_t>(src._seek_time))
    ,_flag(src._flag)
    ,_format_ctx(src._format_ctx)
    ,video(nullptr)
    ,audio(nullptr)
    ,subtitle(nullptr)
    ,_vRead(src._vRead)
    ,_aRead(src._aRead)
    ,_channel(0)
    ,_channelSel(0)
    ,_decodeType(src._decodeType)
    ,m_preview(nullptr)
    ,_audio_thread(src._audio_thread)
    ,_video_thread(nullptr)
{
    init();
    setFlag(flag_bit_Stop, false);
    setFlag(flag_bit_pause, false);
    setFlag(flag_bit_need_pause, false);
    audio->setVol(src.audio->getVol());
    video->setDecodeType(src.video->getDecodeType());
}

core_media::~core_media()
{
    uninit();
    SAFE_RELEASE_PTR(&m_preview)
}

void core_media::init()
{
    INIT_NEW(&video, core_decoder_video)
    INIT_NEW(&audio, core_decoder_audio)
    INIT_NEW(&subtitle, core_decoder_subtitle)
//    INIT_NEW(&_audio_thread, core_thread_audio)
    INIT_NEW(&_video_thread, core_thread_video)
    INIT_NEW(&m_preview, core_preview)
}

void core_media::uninit()
{
    for(int n = 0; n < channel_max; ++n)
    {
        for(auto& it : _channels[n])
        {
            SAFE_RELEASE_PTR(&it)
        }

        _channels[n].clear();
    }

    SAFE_RELEASE_PTR(&video)
    SAFE_RELEASE_PTR(&audio)
    SAFE_RELEASE_PTR(&subtitle)
//    SAFE_RELEASE_PTR(&_audio_thread)
    SAFE_RELEASE_PTR(&_video_thread)
    SAFE_RELEASE_PTR(&m_preview)

    if(_format_ctx)
    {
        avformat_close_input(&_format_ctx);
        avformat_free_context(_format_ctx);
        _format_ctx = nullptr;
    }
}

void core_media::setCallback(video_interface *cb)
{
    _cb = cb;
}

void core_media::setSrc(const std::string &src)
{
    _src = src;
}

void core_media::setSize(int width, int height)
{
    video->setSize(width, height);
}

void core_media::play()
{
    setFlag(flag_bit_Stop, false);
    setFlag(flag_bit_pause, false);
    setFlag(flag_bit_need_pause, false);
}

void core_media::setPause()
{
    Log(Log_Info, "pause: start_time(%.2f)", _start_time/1000000);
    if(!isPause())
        setFlag(flag_bit_need_pause);
}

bool core_media::isPause()
{
    return testFlag(flag_bit_pause);
}

void core_media::continuePlay()
{
    if(isPause())
    {
        auto t = _start_time;
        _start_time += av_gettime() - _pause_time;
        Log(Log_Info, "continue: start_time(%.2f -> %.2f)", t/1000000, _pause_time/1000000);
        setFlag(flag_bit_need_pause, false);
        setFlag(flag_bit_pause, false);
        _state = video_player_core::state_running;
    }
}

void core_media::stop()
{
    setFlag(flag_bit_Stop);
}

bool core_media::isStop()
{
    return testFlag(flag_bit_tvideo_finish) && testFlag(flag_bit_taudio_finish);
}

void core_media::seek(int64_t ms)
{
    if(!testFlag(flag_bit_seek))
    {
        _seek_pos = ms * 1000;
        setFlag(flag_bit_seek);
    }
}

void core_media::preview(int64_t ms)
{
    m_preview->preview(_src, ms, _cb);
}

void core_media::setVol(int nVol)
{
    audio->setVol(nVol);
}

void core_media::mute(bool bMute)
{
    setFlag(flag_bit_mute, bMute);
}

void core_media::setChannel(int channel, int sel)
{
    _channel = channel;
    _channelSel = sel;
    setFlag(flag_bit_channel_change, true);
}

void core_media::setDecode(video_player_core::enum_decode_type type)
{
    _decodeType = type;
    setFlag(flag_bit_decode_change, true);
}

int core_media::state()
{
    return _state;
}

video_player_core::enum_state core_media::state1()
{
    if(testFlag(flag_bit_pause) || testFlag( flag_bit_need_pause))
        return video_player_core::state_paused;
    if(testFlag(flag_bit_Stop))
        return video_player_core::state_stopped;
    return video_player_core::state_running;
}

void av_log_call(void* /*ptr*/, int level, const char* fmt, va_list vl)
{
    if (level <= AV_LOG_INFO)
    {
        char buffer[1024];
        vsprintf(buffer, fmt, vl);
        Log(Log_Info,"msg : [%d] %s", level, buffer);
    }
}

bool core_media::open(int index)
{
    av_log_set_callback(&av_log_call);
    Log(Log_Info, "video info:%p vol:%.2f mute:%d", this, audio->getVol(), testFlag(flag_bit_mute));
    if(!_cb)
    {
        Log(Log_Err, "not set callback!");
        return false;
    }

    int ret = 0, videoIndex = -1, audioIndex = -1, subtitleIndex = -1;
    _cb->startCall(index);
    // allocate context
    _format_ctx = avformat_alloc_context();
    auto& pFormatCtx = _format_ctx;
    Log(Log_Info, "start open input. thread:%d", core_util::getThreadId());
    // open file
    if((ret = avformat_open_input(&pFormatCtx, _src.c_str(), nullptr, nullptr)) < 0)
    {
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Err, "open file failed, [%d,%s]", ret, buf);
        goto media_start_end;
    }

    Log(Log_Info, "start find stream info.name[%s]",pFormatCtx->iformat->name);
    // find stream info
    if((ret = avformat_find_stream_info(pFormatCtx, nullptr)) < 0)
    {
        Log(Log_Err, "find stream info failed, [%d]", ret);
        goto media_start_end;
    }

    Log(Log_Info, "find stream info over.");
    // find video/audio stream index
    int type = -1;

    for(int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        auto stream = pFormatCtx->streams[i];
        auto metadata = stream->metadata;
        std::string sLanguage, sTitle;
        auto language = av_dict_get(metadata, "language", nullptr, AV_DICT_IGNORE_SUFFIX);
        auto title = av_dict_get(metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX);
        if(language)
        {
            sLanguage = language->value;
            Log(Log_Info,"[format: index_%d] %s: %s", i, language->key, language->value);
        }
        if(title)
        {
            sTitle = title->value;
            Log(Log_Info,"[format: index_%d] %s: %s", i, title->key, title->value);
        }

        auto _codec = pFormatCtx->streams[i]->codec;
        Log(Log_Info, "stream[%d]:%d", i, _codec->codec_type);
        type = _codec->codec_type;
        switch (type) {
        case AVMEDIA_TYPE_VIDEO:
            video->pushStream(i);
            videoIndex = i;
            _channels[channel_video].push_back(new _stream_channel_info_(sTitle, sLanguage, i));
            break;
        case AVMEDIA_TYPE_AUDIO:
            audio->pushStream(i);
            _channels[channel_audio].push_back(new _stream_channel_info_(sTitle, sLanguage, i));
            if(audioIndex < 0)
                audioIndex = i;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            subtitle->pushStream(i);
            _channels[channel_subtitle].push_back(new _stream_channel_info_(sTitle, sLanguage, i));
            if(subtitleIndex < 0)
                subtitleIndex = i;
            break;
        }
    }

    _cb->totalTime(pFormatCtx->duration);
    _cb->displayStreamChannelInfo(channel_video, _channels[channel_video], videoIndex);
    _cb->displayStreamChannelInfo(channel_audio, _channels[channel_audio], audioIndex);
    _cb->displayStreamChannelInfo(channel_subtitle, _channels[channel_subtitle], subtitleIndex);

    // init video decoder
    if(!video->init(_format_ctx, videoIndex))
    {
        setFlag(flag_bit_tvideo_finish);
    }

    // init audio decoder
    if(!audio->init(_format_ctx, audioIndex))
    {
        setFlag(flag_bit_taudio_finish);
    }

    // init subtitle decoder
    subtitle->init(_format_ctx, subtitleIndex);

    if(videoIndex < 0 && audioIndex < 0) {
        Log(Log_Err, "[video_audio_index] invalid!", videoIndex, audioIndex);
        goto media_start_end;
    }

    Log(Log_Info, "[default_index] video_index(%d),audio_index(%d),subtitl_index(%d)", videoIndex, audioIndex, subtitleIndex);
    Log(Log_Info, "[total_time] %lld", pFormatCtx->duration);

    // start thread
    if(video->index() >= 0)
        _video_thread->start(this);
    if(audio->index() >= 0)
        _audio_thread->start(this);

    decodeloop();
    return true;
media_start_end:
    Log(Log_Info, "finished.");
    if(_cb)
    {
//        SAFE_RELEASE_PTR(&m_threads[index]);
        _cb->endCall(index);
        OUT_PUT_LEAK();
    }

    return false;
}

int core_media::videoIndex()
{
    return video->index();
}

int core_media::audioIndex()
{
    return audio->index();
}

void core_media::decodeloop()
{
    auto info = this;
    auto& aduio_pks = info->audio->pks;
    auto& video_pks = info->video->pks;

    info->_start_time = static_cast<double>(av_gettime());
    Log(Log_Info, "t_base: %lld", info->_start_time);
    Log(Log_Info, "thread_id:%d", core_util::getThreadId());
    bool bSeek = false;
    for(;;)
    {
        if(testFlag(flag_bit_Stop))
            break;
        if(testFlag(flag_bit_seek) && !bSeek)
        {
            bSeek = true;
            seek();
        }

        if(testFlag(flag_bit_channel_change))
        {
            channelChange();
        }

        if(testFlag(flag_bit_decode_change))
        {
            decodeChange();
        }

        if(aduio_pks.isMax() || video_pks.isMax() || (testFlag(flag_bit_pause) && !bSeek))
        {
            msleep(10);
            continue;
        }

        if(!push_frame(bSeek)) break;
    }

    while(!testFlag(flag_bit_Stop))
    {
        Log(Log_Info, "waitting for stop...");
        msleep(100);
    }
    aduio_pks.clear();
    video_pks.clear();

    if(info->audio->sdl())
    {
        info->audio->sdl()->pauseSDL();
        msleep(100);
    }

    while(!testFlag(flag_bit_tvideo_finish))
    {
        Log(Log_Info, "waitting for video thread stop...");
        msleep(100);
    }
}

void core_media::setFlag(int bit, bool value)
{
    core_util::setBit(_flag, bit, value);
}

bool core_media::testFlag(int bit)
{
    return core_util::isSetBit(_flag, bit);
}

void core_media::setState(int state)
{
    _state = state;
}

void core_media::seek()
{
    auto info = this;
    auto target = info->_seek_pos;
    int sIndex = -1;
    if(info->video->isValid())
        sIndex = info->video->index();
    else if(info->audio->isValid())
        sIndex = info->audio->index();

    AVRational aVRational = {1, AV_TIME_BASE};
    if(sIndex >= 0)
        target = av_rescale_q(target, aVRational, info->_format_ctx->streams[sIndex]->time_base);
    if(av_seek_frame(info->_format_ctx, sIndex, target, AVSEEK_FLAG_BACKWARD) < 0)
        Log(Log_Warning, "%s:seek failed!", info->_format_ctx->filename);
    else{
        cleanPkt();
    }
}

void core_media::cleanPkt(int channel)
{
    msleep(30);
    if(channel < 0)
    {
        video->cleanPkt();
        audio->cleanPkt();
        subtitle->cleanPkt();
    }
    else
    {
        switch (channel) {
        case channel_video:
            video->cleanPkt();
            break;
        case channel_audio:
            audio->cleanPkt();
            break;
        case channel_subtitle:
            subtitle->cleanPkt();
            break;
        }
    }
}

void core_media::channelChange()
{
    auto index = _channels[_channel][_channelSel]->index;
    switch (_channel) {
    case channel_video:
        video->setIndex(index);
        break;
    case channel_audio:
        audio->setIndex(index);
        break;
    case channel_subtitle:
        subtitle->setIndex(index);
        break;
    }

    setFlag(flag_bit_channel_change, false);
}

void core_media::decodeChange()
{
    msleep(30);
    if(video->setDecodeType(_decodeType))
        setFlag(flag_bit_decode_change, false);
}

bool core_media::push_frame(bool &bSeek)
{
    int nRet = 0;
    auto info = this;
    auto& ctx = info->_format_ctx;
    auto& vIndex = info->video->index();
    auto& aIndex = info->audio->index();

    auto& vIndexs = info->video->indexs();
    auto& aIndexs = info->audio->indexs();
    auto& sIndexs = info->subtitle->indexs();

    AVPacket pk;
    if((nRet = av_read_frame(ctx, &pk)) < 0)
    {
        if(!testFlag(flag_bit_read_finish))
        {
            char buf[1024] = {};
            av_strerror(nRet, buf, 1024);
            Log(Log_Warning, "read fram failed,%s, video_pks:%d", buf,info->video->pktSize());
        }

        setFlag(flag_bit_read_finish);
        if(testFlag(flag_bit_Stop))
            return false;
        msleep(10);
        return true;
    }

    if(bSeek)
    {
        if(vIndex != pk.stream_index)
        {
            if(aIndex == pk.stream_index)
            {
                info->audio->setClock(av_q2d(info->audio->stream->time_base) * pk.pts);
            }
        }
        else
        {
            bSeek = checkSeekPkt(&pk);
        }

        av_packet_unref(&pk);
        if(!bSeek)
        {
            Log(Log_Info, "seek: video_pts[%.3f] audio_pts[%.3f]", info->video->clock(), info->audio->clock());
        }

        return true;
    }

    if(vIndexs.find(pk.stream_index) != vIndexs.end() || sIndexs.find(pk.stream_index) != sIndexs.end())
    {
        ++info->_vRead;
        info->video->pks.push_back(pk);
    }
    else if(aIndexs.find(pk.stream_index) != aIndexs.end())
    {
        if(testFlag(flag_bit_taudio_finish))
        {
            av_packet_unref(&pk);
        }
        else
        {
            ++info->_aRead;
            info->audio->pks.push_back(pk);
        }
    }
    else
    {
        av_packet_unref(&pk);
    }


    return true;
}

bool core_media::checkSeekPkt(AVPacket *pk)
{
    if(video->checkSeekPkt(pk))
        return true;

    auto pts = video->clock();
    if(_cb)
    {
        _cb->posChange(static_cast<int64_t>(pts * 1000));
        video->displayFrame(_cb);
    }

    _start_time = static_cast<double>(av_gettime() - _seek_pos);
    setFlag(flag_bit_read_finish, false);

    _seek_time = CALC_PTS(_seek_pos, 0);
    if((testFlag(flag_bit_pause)))
    {
        setFlag(flag_bit_pause, false);
        setFlag(flag_bit_need_pause);
        _pause_time = static_cast<double>(av_gettime());
    }

    setFlag(flag_bit_seek, false);
    return false;
}
