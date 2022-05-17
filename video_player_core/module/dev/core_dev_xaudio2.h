#ifndef CORE_DEV_XAUDIO2_H
#define CORE_DEV_XAUDIO2_H

#include "core_dev.h"

class core_dev_xaudio2Private;
class core_dev_xaudio2 : public core_dev
{
    VP_DECLARE_PRIVATE(core_dev_xaudio2)
public:
    core_dev_xaudio2();
};

#endif // CORE_DEV_XAUDIO2_H
