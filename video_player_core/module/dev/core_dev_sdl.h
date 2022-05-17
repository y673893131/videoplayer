#ifndef CORE_DEV_SDL_H
#define CORE_DEV_SDL_H

#include "core_dev.h"

class core_dev_sdlPrivate;
class core_dev_sdl : public core_dev
{
    VP_DECLARE_PRIVATE(core_dev_sdl)
public:
    core_dev_sdl();
};

#endif // CORE_DEV_SDL_H
