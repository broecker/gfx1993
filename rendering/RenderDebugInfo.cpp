#include "RenderDebugInfo.h"

#include <cstdio>
#include <string>

namespace gfx1993 {
namespace render {

void DebugInfo::reset() {
  points = PrimitiveDebugInfo();
  lines = PrimitiveDebugInfo();
  triangles = PrimitiveDebugInfo();
}

static void printPrimitiveDebugInfo(const std::string& name, const PrimitiveDebugInfo& info) {
  float effective = 0;
  uint32_t total = info.fragmentsDiscarded + info.fragmentsDrawn;
  if (total > 0) {
    effective = static_cast<float>(info.fragmentsDrawn) / (info.fragmentsDrawn + info.fragmentsDiscarded);
  }
  printf("%10s:    %6u primitives; fragments: %10u total, %10u drawn %10u discarded; effective: %5.2f\n", name.c_str(), info.drawn, total, info.fragmentsDrawn, info.fragmentsDiscarded, effective);
}

void DebugInfo::print() const {
  printf("Rasterizer debug draw counters:\n");
  printPrimitiveDebugInfo("Points", points);
  printPrimitiveDebugInfo("Lines", lines);
  printPrimitiveDebugInfo("Triangles", triangles);
}

}  // namespace render
}  // namespace gfx1993