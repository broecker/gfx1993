#include "RenderDebugInfo.h"

#include <cstdio>
#include <string>

namespace gfx1993 {
namespace util {

void DebugInfo::reset() {
  points = PrimitiveDebugInfo{.processed=0,.drawn=0,.backfaceCulled=0,.fragmentsDrawn=0,.fragmentsDiscarded=0};
  lines = PrimitiveDebugInfo{.processed=0,.drawn=0,.backfaceCulled=0,.fragmentsDrawn=0,.fragmentsDiscarded=0};
  triangles = PrimitiveDebugInfo{.processed=0,.drawn=0,.backfaceCulled=0,.fragmentsDrawn=0,.fragmentsDiscarded=0};
  screenFillingQuad = PrimitiveDebugInfo{.processed=0,.drawn=0,.backfaceCulled=0,.fragmentsDrawn=0,.fragmentsDiscarded=0};
  aabbs = PrimitiveDebugInfo{.processed=0,.drawn=0,.backfaceCulled=0,.fragmentsDrawn=0,.fragmentsDiscarded=0};
}

static void printPrimitiveDebugInfo(const std::string& name, const PrimitiveDebugInfo& info) {
  float effective = 0;
  uint32_t total = info.fragmentsDiscarded + info.fragmentsDrawn;
  if (total > 0) {
    effective = static_cast<float>(info.fragmentsDrawn) / (info.fragmentsDrawn + info.fragmentsDiscarded);
  }
  printf("%10s:    %6u primitives; %6u backfaceCulled; fragments: %10u total, %10u drawn %10u discarded; effective: %5.2f\n", name.c_str(), info.drawn, info.backfaceCulled, total, info.fragmentsDrawn, info.fragmentsDiscarded, effective);
}

void DebugInfo::print() const {
  printf("Rasterizer debug draw counters:\n");
  printPrimitiveDebugInfo("Points", points);
  printPrimitiveDebugInfo("Lines", lines);
  printPrimitiveDebugInfo("Triangles", triangles);
  printPrimitiveDebugInfo("ScreenQuad", screenFillingQuad);
  printPrimitiveDebugInfo("AABBs", aabbs);
}

}  // namespace util
}  // namespace gfx1993