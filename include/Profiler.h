#ifndef GFX1993_PROFILER_INCLUDED
#define GFX1993_PROFILER_INCLUDED

#include "config.h"

#if GFX1993_ENABLE_PROFILING
  #include <tracy/Tracy.hpp>
  #define GFX1993_ZONE() ZoneScoped
  #define GFX1993_ZONE_N(name) ZoneScopedN(name)
  #define GFX1993_FRAME_MARK() FrameMark
#else
  #define GFX1993_ZONE()
  #define GFX1993_ZONE_N(name)
  #define GFX1993_FRAME_MARK()
#endif // GFX1993_ENABLE_PROFILING

#endif // GFX1993_PROFILER_INCLUDED
