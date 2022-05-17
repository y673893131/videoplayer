#ifndef CORE_DEV_WAVEOUT_H
#define CORE_DEV_WAVEOUT_H

#ifdef WIN32
#include "core_dev.h"

class core_dev_waveoutPrivate;
class core_dev_waveout : public core_dev
{
    VP_DECLARE_PRIVATE(core_dev_waveout)
public:
    core_dev_waveout();
    void defaultChanged();
};

#endif
#endif // CORE_DEV_WAVEOUT_H
