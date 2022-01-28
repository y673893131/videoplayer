#ifndef CORE_THREAD_SUBTITLE_H
#define CORE_THREAD_SUBTITLE_H

#include "../common.h"
#include "core_thread.h"
class core_thread_subtitle : public core_thread
{
public:
    core_thread_subtitle();
    virtual ~core_thread_subtitle() override;

    bool start(core_media* media) override;
private:
    void threadCall();
};

#endif // CORE_THREAD_SUBTITLE_H
