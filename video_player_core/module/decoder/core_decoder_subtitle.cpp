#include "core_decoder_subtitle.h"

core_decoder_subtitle::core_decoder_subtitle()
    : pSubtitle(nullptr)
{

}

core_decoder_subtitle::~core_decoder_subtitle()
{
    uninit();
}

bool core_decoder_subtitle::init(AVFormatContext *formatCtx, int index)
{
    uninit();
    core_decoder::init(formatCtx, index);
    INIT_NEW(&pSubtitle, AVSubtitle)
    return true;
}

void core_decoder_subtitle::uninit()
{
    core_decoder::uninit();
    SAFE_RELEASE_PTR(&pSubtitle)
}

AVSubtitle *core_decoder_subtitle::subtitle()
{
    return pSubtitle;
}
