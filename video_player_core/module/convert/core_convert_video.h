#ifndef CORE_CONVERT_VIDEO_H
#define CORE_CONVERT_VIDEO_H

#include "core_convert.h"

class core_convert_videoPrivate;
class core_convert_video : public core_convert
{
    VP_DECLARE_PRIVATE(core_convert_video)
public:
    core_convert_video();
};

#endif // CORE_CONVERT_VIDEO_H
