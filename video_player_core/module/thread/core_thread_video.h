#ifndef CORE_THREAD_VIDEO_H
#define CORE_THREAD_VIDEO_H

#include "../common.h"
#include "core_thread.h"
class core_thread_video : public core_thread
{
public:
    core_thread_video();
    virtual ~core_thread_video() override;

    bool start(core_media* media) override;
private:
    void threadCall();
};

#endif // CORE_THREAD_VIDEO_H
