//
// Created by mbroecker on 4/29/20.
//

#ifndef GFX1993_RENDERDEBUGINFO_H
#define GFX1993_RENDERDEBUGINFO_H

namespace render
{

// Contains rasterization debug information.
struct DebugInfo
{
  // Rasterization information.
  int pointsDrawn = 0;
  int linesDrawn = 0;
  int trianglesDrawn = 0;



  inline void reset()
  {
    pointsDrawn = 0;
    linesDrawn = 0;
    trianglesDrawn = 0;
  }

};

}

#endif // GFX1993_RENDERDEBUGINFO_H
