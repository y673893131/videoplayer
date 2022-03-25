#include "core_decoder_subtitle.h"

core_decoder_subtitle::core_decoder_subtitle()
    : pSubtitle(nullptr)
    , _ptsEnd(0)
{

}

core_decoder_subtitle::~core_decoder_subtitle()
{
    uninit();
}

bool core_decoder_subtitle::init(AVFormatContext *formatCtx, int index)
{
    uninit();
    if(!core_decoder::init(formatCtx, index))
        return false;

    INIT_NEW(&pSubtitle, AVSubtitle)

    //AV_CODEC_ID_ASS
//    auto subHeader = std::string(reinterpret_cast<char*>(pCodecContext->subtitle_header), static_cast<unsigned int>(pCodecContext->subtitle_header_size));
//    Log(Log_Debug, subHeader.c_str());

    return true;
}

void core_decoder_subtitle::uninit()
{
    core_decoder::uninit();
    SAFE_RELEASE_PTR(&pSubtitle)
}

bool core_decoder_subtitle::decode(AVPacket *pk, bool &/*bTryAgain*/)
{
    int got = -1, ret = 0;
    ret = avcodec_decode_subtitle2(pCodecContext, pSubtitle, &got, pk);
    if(got > 0)
    {
        auto pts = static_cast<int64_t>(pSubtitle->pts / static_cast<double>(AV_TIME_BASE) * 1000);
        setClock(pts + pSubtitle->start_display_time);
        _ptsEnd = pts + pSubtitle->end_display_time;

        return true;
    }

    avsubtitle_free(pSubtitle);
    return false;
}

AVSubtitle *core_decoder_subtitle::subtitle()
{
    return pSubtitle;
}

void core_decoder_subtitle::subtitleHeader(video_interface *cb)
{
    if(!pCodecContext || !pCodecContext->subtitle_header || pCodecContext->subtitle_header_size <= 0)
        return;

    auto sHeader = std::string(reinterpret_cast<char*>(pCodecContext->subtitle_header), static_cast<unsigned int>(pCodecContext->subtitle_header_size));
    subtitle_header header;
    switch (pCodec->id) {
    case AV_CODEC_ID_ASS:
        analyzeAASHeader(header, sHeader);
        break;
    default:
    {
        header.bInit = true;
        subtitle_info info = {};
        _snprintf(info.sFont, sizeof(info.sFont), "Arial");
        info.pt = 18;
        info.bold = 1;
        info.color[0] = 0xFFFFFFFF;
        header.infos.push_back(info);
    }return;
    }

    header.sHeader = sHeader;
    cb->subtitleHaderCall(header);
}

void core_decoder_subtitle::displayFrame(video_interface *cb, int64_t startTime)
{
    char* subtitleString = nullptr;
    unsigned int number = pSubtitle->num_rects;

    auto start = displayClock() - startTime;
    auto end = getDisplayPts(_ptsEnd) - startTime;
    for (unsigned int i = 0; i < number; ++i)
    {
        auto rc = pSubtitle->rects[i];
        switch (rc->type) {
        case SUBTITLE_ASS:
            subtitleString = rc->ass;
            break;
        case SUBTITLE_TEXT:
            subtitleString = rc->text;
            break;
        }

        cb->displaySubTitleCall(subtitleString, i, rc->type, start, end);
    }

    avsubtitle_free(pSubtitle);
}

void core_decoder_subtitle::analyzeAASHeader(subtitle_header &header, const std::string &desc)
{
    int pos_begin = 0, pos_end = 0;
    while((pos_begin = desc.find("\r\nStyle: ", pos_begin)) > 0)
    {
        pos_begin += 2;
        pos_end = desc.find("\r\n", pos_begin);

        auto line = desc.substr(pos_begin, pos_end - pos_begin);
        LogB(Log_Debug, "%s", line.c_str());
        try {
            subtitle_info info;
            memset(&info, 0x00, sizeof(subtitle_info));
            if(0 <= sscanf(line.c_str(), "Style: "
                                   "%[^,],"                  /* Name */
                                   "%[^,],%d,"               /* Font{name,size} */
                                   "&H%x,&H%x,&H%x,&H%x," /* {Primary,Secondary,Outline,Back}Colour */
                                   "%d,%d,%d,%d,"          /* Bold, Italic, Underline, StrikeOut */
                                   "%d,%d,"                /* Scale{X,Y} */
                                   "%d,%d,"                 /* Spacing, Angle */
                                   "%d,%d,%d,"              /* BorderStyle, Outline, Shadow */
                                   "%d,%d,%d,%d,"         /* Alignment, Margin[LRV] */
                                   "%d"                /* Encoding */
                    , info.sName
                    , info.sFont, &info.pt
                    , &info.color[0], &info.color[1], &info.color[2], &info.color[3]
                    , &info.bold, &info.italic, &info.underLine, &info.strikeOut
                    , &info.scaleX, &info.scaleY
                    , &info.spacing, &info.angle
                    , &info.borderStyle, &info.outlinePix, &info.shadowPix
                    , &info.alignment, &info.marginLeft, &info.marginRight, &info.marginBottom
                    , &info.encoding)
            ){
                header.bInit = true;
                header.isASS = true;
                header.infos.push_back(info);
            }

        } catch (...) {
            Log(Log_Err, "parse aas error: %s", line.c_str());
        }

        pos_begin = pos_end;
    }
}
