#ifndef CORE_DEV_DSOUND_H
#define CORE_DEV_DSOUND_H

#include "core_dev.h"

class core_dev_dsoundPrivate;
class core_dev_dsound : public core_dev
{
    VP_DECLARE_PRIVATE(core_dev_dsound)
public:
    core_dev_dsound();
};

#endif // CORE_DEV_DSOUND_H
