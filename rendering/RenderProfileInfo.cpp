#include "RenderProfileInfo.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <limits>

using namespace render;

static uint32_t getMilliseconds() {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();\
}

ProfileMarker RenderProfile::startTiming(const std::string& name) const {
  return ProfileMarker{.name=name, .startTime=getMilliseconds()};
}

void RenderProfile::endTiming(const ProfileMarker& marker) {
  uint32_t duration = getMilliseconds() - marker.startTime;

  auto it = stats.find(marker.name);
  if (it == stats.end()) {
    stats[marker.name] = {.samples={duration}};
  } else {
    it->second.add(duration);
  }
}

void RenderProfile::print() const {
  printf("Rasterizer profile:\n");

  // We have a key hierarchy; e.g. app.blit.copy
  // which we want to display/expand as:
  // app       ....
  //   .blit   ....
  //     .copy ....
  // TODO: auto aggregate higher ups. 
  // We should still be able to rely on the key sorting
  // to get the initial hierarchy.

  for (const auto& stat : stats) {
    uint32_t min = std::numeric_limits<uint32_t>::max();
    uint32_t max = std::numeric_limits<uint32_t>::min();
    uint32_t sum = 0;
    for (uint32_t s : stat.second.samples) {
      sum += s;
      min = std::min(s, min);
      max = std::max(s, max);
    }

    float mean = static_cast<float>(sum) / stat.second.samples.size();
    
    printf("%-28s: %5.2fms\t[%5d-%5d]; %lu samples\n", stat.first.c_str(), mean, min, max, stat.second.samples.size());
  } 
}