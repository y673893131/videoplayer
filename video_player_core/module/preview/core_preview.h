#ifndef CORE_PREVIEW_H
#define CORE_PREVIEW_H

#include <video_pimpl.h>
#include "video_player_core.h"

class core_previewPrivate;
class core_preview
{
    VP_DECLARE(core_preview)
    VP_DECLARE_PRIVATE(core_preview)
public:
    core_preview();
    virtual ~core_preview();

    void setCallBack(video_interface* cb);
    void preview(const std::string& src, int64_t ms, int flag);
};

#endif // CORE_PREVIEW_H
