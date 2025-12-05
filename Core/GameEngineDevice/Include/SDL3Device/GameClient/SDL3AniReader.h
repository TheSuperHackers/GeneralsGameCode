#pragma once

#include "Lib/BaseType.h"

#include <SDL3/SDL.h>

#include <vector>

// TheSuperHackers @feature denysmitin 05/12/2025 SDL3 ANI reader interface for portable animated cursors.
class SDL3AniReader
{
public:
  struct Frame
  {
    Frame() : cursor(NULL), durationMs(0), hotspotX(0), hotspotY(0) {}

    SDL_Cursor *cursor;
    UnsignedInt durationMs;
    Int hotspotX;
    Int hotspotY;
  };

  SDL3AniReader();
  ~SDL3AniReader();

  Bool load(const char *path, std::vector<Frame> &outFrames);

private:
  SDL3AniReader(const SDL3AniReader &);
  SDL3AniReader &operator=(const SDL3AniReader &);
};
#pragma once
