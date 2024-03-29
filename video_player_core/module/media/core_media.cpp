#include "core_media.h"
#include "../util/core_util.h"
#include "../filter/core_filter_audio.h"
#include "../dev/core_dev.h"

core_media::core_media()
    :_cb(nullptr)
    ,_state(0)
    ,_start_time(0)
//    ,_seek_time(0)
    ,_pause_time(0)
    ,_seek_pos(0)
    ,_seek_step(0)
    ,_flag(0)
    ,_format_ctx(nullptr)
    ,_video(nullptr)
    ,_audio(nullptr)
    ,_subtitle(nullptr)
    ,_channel(0)
    ,_channelSel(0)
    ,_decodeType(0)
    ,_speed(0)
    ,_capture(false)
    ,m_preview(nullptr)
    ,_video_thread(nullptr)
    ,_subtitle_thread(nullptr)
    ,_save(nullptr)
{
    setFlag(flag_bit_Stop);
    _audio_thread = core_thread_audio::instance();
}

core_media::core_media(const core_media &src)
    :_cb(src._cb)
    ,_state(src._state)
    ,_src(src._src)
    ,_start_time(src._start_time)
//    ,_seek_time(static_cast<double>(src._seek_pos))
    ,_pause_time(src._pause_time)
    ,_seek_pos(static_cast<int64_t>(src._seek_pos))
    ,_seek_step(src._seek_step)
    ,_flag(src._flag)
    ,_format_ctx(src._format_ctx)
    ,_video(nullptr)
    ,_audio(nullptr)
    ,_subtitle(nullptr)
    ,_channel(0)
    ,_channelSel(0)
    ,_decodeType(src._decodeType)
    ,_speed(src._speed)
    ,_capture(src._capture)
    ,m_preview(nullptr)
    ,_audio_thread(src._audio_thread)
    ,_video_thread(nullptr)
    ,_subtitle_thread(nullptr)
    ,_save(nullptr)
{
    init();
    setFlag(flag_bit_Stop, false);
    setFlag(flag_bit_pause, false);
    setFlag(flag_bit_need_pause, false);
    _audio->setVol(src._audio->getVol());
    _video->setDecodeType(src._video->getDecodeType());
#ifdef AUDIO_FILTER
    dynamic_cast<core_filter_audio*>(_audio->m_filter)->setAtempo(_speed);
#endif
}

core_media::~core_media()
{
    uninit();
    SAFE_RELEASE_PTR(&m_preview)
}

void core_media::init()
{
    INIT_NEW(&_video, core_decoder_video)
    INIT_NEW(&_audio, core_decoder_audio)
    INIT_NEW(&_subtitle, core_decoder_subtitle)
//    INIT_NEW(&_audio_thread, core_thread_audio)
    INIT_NEW(&_video_thread, core_thread_video)
    INIT_NEW(&_subtitle_thread, core_thread_subtitle)
    INIT_NEW(&m_preview, core_preview)
    INIT_NEW(&_save, core_save)
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

    SAFE_RELEASE_PTR(&_video)
    SAFE_RELEASE_PTR(&_audio)
    SAFE_RELEASE_PTR(&_subtitle)
//    SAFE_RELEASE_PTR(&_audio_thread)
    SAFE_RELEASE_PTR(&_video_thread)
    SAFE_RELEASE_PTR(&_subtitle_thread)
    SAFE_RELEASE_PTR(&m_preview)
    SAFE_RELEASE_PTR(&_save)

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

void core_media::play()
{
    setFlag(flag_bit_Stop, false);
    setFlag(flag_bit_pause, false);
    setFlag(flag_bit_need_pause, false);
    _audio->start();
}

void core_media::setPause()
{
//    Log(Log_Info, "pause: start_time(%.2f)", _start_time/1000000);
    if(!isPause())
    {
//        setFlag(flag_bit_need_pause);
        setFlag(flag_bit_pause, true);
        setState(video_player_core::state_paused);
//        if(_audio->dev()) _audio->dev()->pause();
    }
}

bool core_media::isPause()
{
    return testFlag(flag_bit_pause);
}

void core_media::continuePlay()
{
    if(isPause())
    {
//        auto t = _start_time;
        _start_time += av_gettime() - _pause_time;
//        Log(Log_Info, "continue: start_time(%.2f -> %.2f)", t/1000000, _pause_time/1000000);

        setFlag(flag_bit_pause, false);
        _audio->start();
        setState(video_player_core::state_running);
    }
}

void core_media::stop()
{
    _audio->pause();
    setFlag(flag_bit_Stop);
}

bool core_media::isStop()
{
    return testFlag(flag_bit_tvideo_finish) && testFlag(flag_bit_taudio_finish);
}

bool core_media::seek(int64_t ms)
{
    if(testFlag(flag_bit_flush))
        return false;
    if(!testFlag(flag_bit_seek))
    {
        _seek_pos = ms * 1000;
        _seek_step = 0;
        setFlag(flag_bit_seek);

        return true;
    }

    return false;
}

bool core_media::jump(int64_t ms)
{
    if(testFlag(flag_bit_flush))
        return false;
    if(!testFlag(flag_bit_seek))
    {
        _seek_pos = 0;
        _seek_step = ms * 1000;
        setFlag(flag_bit_seek);
        return true;
    }

    return false;
}

void core_media::preview(int64_t ms)
{
    if(!_video)
        return;

    m_preview->setCallBack(_cb);
    m_preview->preview(_src, ms, getSeekFlag(_video, _video->getInteralPts(ms * 1000 + start_time())));
}

void core_media::setVol(int nVol)
{
    _audio->setVol(nVol);
}

void core_media::mute(bool bMute)
{
    setFlag(flag_bit_mute, bMute);
}

void core_media::setAudioChannel(int type)
{
    _audio->setAudioChannel(type);
}

void core_media::setChannel(int channel, int sel)
{
    _channel = static_cast<unsigned int>(channel);
    _channelSel = static_cast<unsigned int>(sel);
    setFlag(flag_bit_channel_change, true);
}

void core_media::setDecode(int type)
{
    _decodeType = type;
    setFlag(flag_bit_decode_change, true);
}

void core_media::setSpeed(int type)
{
    _speed = type;
    dynamic_cast<core_filter_audio*>(_audio->m_filter)->setAtempo(_speed);
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

int core_media::setCapture(bool bCap)
{
    _capture = bCap;
    if(bCap)
    {
        _save->start();
        setFlag(flag_bit_save, bCap);
    }
    else
    {
        setFlag(flag_bit_save, bCap);
        msleep(10);
        _save->stop();
    }

    return 0;
}

void av_log_call(void* /*ptr*/, int level, const char* fmt, va_list vl)
{
    if (level < AV_LOG_INFO)
    {
        char buffer[1024];
#ifdef WIN32
//        int size = _vsnprintf(nullptr, 0, fmt, vl);
        _vsnprintf_s(buffer, 1024, fmt, vl);
#else
        _snprintf(buffer, 1024, fmt, vl);
#endif
        LogB(Log_Debug,"[ffmpeg:%d] %s", level, buffer);
    }
}

int core_media::interruptCallback(void* para)
{
    if(!para)
        return 0;
    auto pThis = reinterpret_cast<core_media*>(para);
    if(pThis->testFlag(flag_bit_Stop))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool core_media::open(int index)
{
    av_log_set_level(AV_LOG_INFO);
    av_log_set_callback(&av_log_call);
    Log(Log_Info, "video info:%p vol:%d mute:%d", this, _audio->getVol(), testFlag(flag_bit_mute));
    if(!_cb)
    {
        Log(Log_Err, "not set callback!");
        return false;
    }

    int ret = 0, streamsCount = 0, videoIndex = -1, audioIndex = -1, subtitleIndex = -1;
    _cb->startCall(index);
    // allocate context
    _format_ctx = avformat_alloc_context();
    _format_ctx->interrupt_callback.callback = interruptCallback;
    _format_ctx->interrupt_callback.opaque = this;
    auto& pFormatCtx = _format_ctx;
    AVCodecContext* codec = nullptr;
    Log(Log_Info, "start open input. thread:%u", core_util::getThreadId());
    // open file

    AVInputFormat *ifmt = nullptr;
    AVDictionary* options = nullptr;
#ifdef USE_DESKTOP
    _src = "desktop";
    #ifdef WIN32
        ifmt = av_find_input_format("gdigrab");
    #elif defined linux
        ifmt = av_find_input_format("x11grab");
    #else
        ifmt = av_find_input_format("avfoundation");//MAC
    #endif
#elif defined USE_CAMERA
    _src = "video=Full HD webcam";
    ifmt = av_find_input_format("dshow");

    av_dict_set(&options, "input_format", "mjpeg", 0);
//    av_dict_set(&options, "video_size", "1920x1080", 0);
//    av_dict_set(&options, "framerate", "25", 0);
#endif
    char error[1024] = {};
    ret = avformat_open_input(&pFormatCtx, _src.c_str(), ifmt, &options);
    if(ret < 0)
    {
        av_strerror(ret, error, 1024);
        Log(Log_Err, "open file failed, [%d,%s] ifmt:%p", ret, error, ifmt);
        goto media_start_end;
    }

    Log(Log_Info, "start find stream info.name[%s]",pFormatCtx->iformat->name);
    // find stream info
    if((ret = avformat_find_stream_info(pFormatCtx, nullptr)) < 0)
    {
        av_strerror(ret, error, 1024);
        Log(Log_Err, "find stream info failed, [%d,%s]", ret, error);
        goto media_start_end;
    }

    Log(Log_Info, "find stream info over. bitrate[%lld]", pFormatCtx->bit_rate);
    // find video/audio stream index

//    AVDictionaryEntry* tmp = nullptr;
//    while((tmp = av_dict_get(pFormatCtx->metadata, "", tmp, AV_DICT_IGNORE_SUFFIX)) != nullptr)
//    {
//        LogB(Log_Debug, "[meta] %s: %s", tmp->key, tmp->value);
//    }

    streamsCount = static_cast<int>(pFormatCtx->nb_streams);
    for(int i = 0; i < streamsCount; ++i)
    {
        auto stream = pFormatCtx->streams[i];
        auto metadata = stream->metadata;

//        std::string sMeta;
        std::string sLanguage, sTitle;
        auto language = av_dict_get(metadata, "language", nullptr, AV_DICT_IGNORE_SUFFIX);
        auto title = av_dict_get(metadata, "title", nullptr, AV_DICT_IGNORE_SUFFIX);
//        auto album = av_dict_get(metadata, "album", nullptr, AV_DICT_IGNORE_SUFFIX);// 专辑
//        auto date = av_dict_get(metadata, "date", nullptr, AV_DICT_IGNORE_SUFFIX);// 日期
//        auto artist = av_dict_get(metadata, "artist", nullptr, AV_DICT_IGNORE_SUFFIX);// 艺人
        if(language)
        {
            sLanguage = language->value;
//            sMeta += std::string("[language]") + sLanguage;
            Log(Log_Info,"[format: index_%d] %s: %s", i, language->key, language->value);
        }
        if(title)
        {
            sTitle = title->value;
//            sMeta += std::string("[title]") + sTitle;
            Log(Log_Info,"[format: index_%d] %s: %s", i, title->key, title->value);
        }
//#define APPEND_META(x) \
//    if(x)\
//    {\
//        sMeta += std::string(#x) + x->value; \
//    }

//        APPEND_META(album)
//        APPEND_META(date)
//        APPEND_META(artist)

//        LogB(Log_Debug, sMeta.c_str());
        codec = stream->codec;
        Log(Log_Info, "stream[%d]:%d", i, codec->codec_type);
        switch (codec->codec_type) {
        case AVMEDIA_TYPE_VIDEO:
            _video->pushStream(static_cast<unsigned int>(i));
            videoIndex = i;
            _channels[channel_video].push_back(new _stream_channel_info_(sTitle, sLanguage, i));
            break;
        case AVMEDIA_TYPE_AUDIO:
            _audio->pushStream(static_cast<unsigned int>(i));
            _channels[channel_audio].push_back(new _stream_channel_info_(sTitle, sLanguage, i));
            if(audioIndex < 0)
                audioIndex = i;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            _subtitle->pushStream(static_cast<unsigned int>(i));
            _channels[channel_subtitle].push_back(new _stream_channel_info_(sTitle, sLanguage, i));
            if(subtitleIndex < 0)
                subtitleIndex = i;
            break;
        }
    }

    _cb->totalTime(pFormatCtx->duration, _src.c_str());
    _cb->displayStreamChannelInfo(channel_video, _channels[channel_video], videoIndex);
    _cb->displayStreamChannelInfo(channel_audio, _channels[channel_audio], audioIndex);
    _cb->displayStreamChannelInfo(channel_subtitle, _channels[channel_subtitle], subtitleIndex);

    // init video decoder
//    _video->initDecodeType(_decodeType);
    if(!_video->init(_format_ctx, videoIndex))
    {
        setFlag(flag_bit_tvideo_finish);
    }

    // init audio decoder
    if(!_audio->init(_format_ctx, audioIndex))
    {
        setFlag(flag_bit_taudio_finish);
    }

    // init subtitle decoder
    if(!_subtitle->init(_format_ctx, subtitleIndex))
    {
        setFlag(flag_bit_tsubtitle_finish);
    }

    if(videoIndex < 0 && audioIndex < 0) {
        av_strerror(ret, error, 1024);
        sprintf(error, "no stream");
        Log(Log_Err, "[video_audio_index] invalid!", videoIndex, audioIndex);
        goto media_start_end;
    }

    Log(Log_Info, "[default_index] video_index(%d),audio_index(%d),subtitl_index(%d)", videoIndex, audioIndex, subtitleIndex);
    Log(Log_Info, "[total_time] %lld", pFormatCtx->duration);

    // start thread
    if(_video->index() >= 0)
        _video_thread->start(this);
    if(_audio->index() >= 0)
        _audio_thread->start(this);
    if(_subtitle->index() >= 0)
        _subtitle_thread->start(this);

    // save init
    _save->init(this, videoIndex, audioIndex);
    if(_capture)
    {
        Log(Log_Info, "[save] start save with init");
        setCapture(true);
        _save->start();
    }

    if(_video->index() >= 0 || _audio->index() >= 0)
        setState(video_player_core::state_running);
    decodeloop();
    if(_capture)
    {
        _save->stop();
    }
    if(_audio->isValid())
        _audio->pause();
    Log(Log_Info, "down.");
    return true;
media_start_end:
    Log(Log_Info, "finished.");
    if(_cb)
    {
//        SAFE_RELEASE_PTR(&m_threads[index]);
        _cb->exceptionEndCall(index, error);
        OUT_PUT_LEAK();
    }

    return false;
}

int core_media::videoIndex()
{
    return _video->index();
}

int core_media::audioIndex()
{
    return _audio->index();
}

void core_media::decodeloop()
{
    auto info = this;
    auto& aduio_pks = info->_audio->pks;
    auto& video_pks = info->_video->pks;
    auto& subtitle_pks = info->_subtitle->pks;

    info->_start_time = av_gettime();
    Log(Log_Info, "t_base: %f", info->_start_time);
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

        if(aduio_pks.isMax() || video_pks.isMax() || subtitle_pks.isMax() || (testFlag(flag_bit_pause) && !bSeek))
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
    subtitle_pks.clear();
//    if(info->_audio->sdl())
//    {
//        info->_audio->sdl()->pauseSDL();
//    }

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
    core_decoder* decoder = nullptr;
    if(info->_video->isValid())
        decoder = info->_video;
    else if(info->_audio->isValid())
        decoder = info->_audio;
    else
        return;

    int64_t start_time = this->start_time();
    auto target = info->_seek_pos;
    int64_t step = 0;
    auto cur = decoder->clock();
    if(decoder)
    {
        if(!info->_seek_step)
        {
            target = decoder->getInteralPts(target + start_time);
        }
        else
        {
            step = decoder->getInteralPts(info->_seek_step + start_time);
            target = cur + step;
            info->_seek_pos = decoder->getDisplayPts(target) * 1000;
        }
    }

    if(target < 0) target = 0;
    auto curDisply = decoder->getDisplayPts(cur);
    auto targetDisplay = decoder->getDisplayPts(target);
    auto sCur = core_util::toTime(curDisply);
    auto sTarget = core_util::toTime(targetDisplay);
//    LogB(Log_Debug, "jump: %s->%s beg", sCur.c_str(), sTarget.c_str());
    int flag = getSeekFlag(decoder, target);

    LogB(Log_Debug, "jump: %lld(start_time:%lld)->%lld beg[flag=%d]", cur, start_time, target, flag);
    if(av_seek_frame(info->_format_ctx, decoder->index(), target, flag) < 0)
    {
        Log(Log_Warning, "%s:seek failed!", info->_format_ctx->url);
    }

    else{
        if(testFlag(flag_bit_read_finish))
            setFlag(flag_bit_read_finish, false);
        pushSeekPkt();
    }
}

int core_media::getSeekFlag(core_decoder* decoder, int64_t target)
{
    return AVSEEK_FLAG_BACKWARD;
    /*
     *  QSV bug must be backward,
     */
    if(_video->getDecodeType() == AV_HWDEVICE_TYPE_QSV)
        return AVSEEK_FLAG_BACKWARD;

    if(!decoder)
        return 0;

    return target < decoder->clock() ? AVSEEK_FLAG_BACKWARD : 0;
}

void core_media::pushSeekPkt()
{
    _video->pks.push_flush();
    _audio->pks.push_flush();
    _subtitle->pks.push_flush();
}

void core_media::channelChange()
{
    auto index = _channels[_channel][_channelSel]->index;
    switch (_channel) {
    case channel_video:
        _video->setIndex(index);
        break;
    case channel_audio:
        _audio->setIndex(index);
        break;
    case channel_subtitle:
        _subtitle->setIndex(index);
        break;
    }

    setFlag(flag_bit_channel_change, false);
}

bool core_media::push_frame(bool &bSeek)
{
    int nRet = 0;
    auto info = this;
    auto& ctx = info->_format_ctx;
    auto& vIndex = info->_video->index();
    auto& aIndex = info->_audio->index();
    auto& sIndex = info->_subtitle->index();

    auto& vIndexs = info->_video->indexs();
    auto& aIndexs = info->_audio->indexs();
    auto& sIndexs = info->_subtitle->indexs();

    AVPacket pk;
//    auto clock1 = clock();
    if((nRet = av_read_frame(ctx, &pk)) < 0)
    {
//        if(nRet == AVERROR_EOF)
//        {
//            avio_seek(ctx->pb, 0, SEEK_SET);
//            avformat_seek_file(ctx, -1, 0, 0, 0, 0);
//            _start_time = av_gettime() - _seek_pos;
//            return true;
//        }
        if(!testFlag(flag_bit_read_finish))
        {
            char buf[1024] = {};
            av_strerror(nRet, buf, 1024);
            Log(Log_Warning, "read frame failed,%s, video_pks:%d audio_pks:%d", buf,info->_video->pktSize(), info->_audio->pktSize());
        }

        setFlag(flag_bit_read_finish);
        if(testFlag(flag_bit_Stop))
            return false;
        msleep(10);
        return true;
    }
//    auto clock2 = clock();
//    if(clock2 - clock1 > 2)
//        LogB(Log_Debug, "push_frame: %d [%d]", clock2 - clock1, info->_audio->pks.size());
    if(bSeek)
    {
        if(vIndex >= 0)
        {
            if(vIndex == pk.stream_index)
            {
                bSeek = checkSeekPkt(info->_video, &pk);
            }
        }
        else if(aIndex == pk.stream_index)
        {
            bSeek = checkSeekPkt(info->_audio, &pk);
        }

        if(bSeek)
        {
            av_packet_unref(&pk);
            return true;
        }
    }


    auto index = static_cast<unsigned int>(pk.stream_index);
    bool bVideo = vIndexs.find(index) != vIndexs.end();
    bool bAudio = aIndexs.find(index) != aIndexs.end();
    bool bSubtitle = sIndexs.find(index) != sIndexs.end();

    if(bVideo)
    {
        info->_video->pks.push_back(pk);
    }
    else if(bAudio)
    {
        info->_audio->pks.push_back(pk);
    }
    else if(bSubtitle)
    {
        info->_subtitle->pks.push_back(pk);
    }
    else
    {
        av_packet_unref(&pk);
    }


    return true;
}

bool core_media::checkSeekPkt(core_decoder* decoder, AVPacket *pk)
{
    if(decoder->checkSeekPkt(pk))
        return true;

    _start_time = av_gettime() - _seek_pos;
    setFlag(flag_bit_read_finish, false);

    if((testFlag(flag_bit_pause)))
    {
        setFlag(flag_bit_pause, false);
        setFlag(flag_bit_need_pause);
        if(_audio->dev())
        {
            _audio->dev()->reset();
        }
        _pause_time = av_gettime();
    }

    setFlag(flag_bit_seek, false);
    setFlag(flag_bit_flush);
//    Log(Log_Debug, "seek: video_pts[%.3f] audio_pts[%.3f]", _video->clock(), _audio->clock());
    return false;
}

int64_t core_media::start_time()
{
    if(!_format_ctx || _format_ctx->start_time == AV_NOPTS_VALUE)
        return 0;
    return _format_ctx->start_time;
}
