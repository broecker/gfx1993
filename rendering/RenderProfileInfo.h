#ifndef GFX1993_RENDERPROFILE_INCLUDED
#define GFX1993_RENDERPROFILE_INCLUDED

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace render {

struct Stat {
  std::vector<uint32_t> samples;

  void add(uint32_t duration) { samples.push_back(duration); }
};

struct ProfileMarker {
  std::string name;
  uint32_t    startTime;
};

struct RenderProfile {
  std::map<std::string, Stat> stats;

  void reset() { stats.clear(); }

  ProfileMarker startTiming(const std::string& name) const;
  void endTiming(const ProfileMarker& marker); 

  void print() const;
};

}  // namespace render

#endif // GFX1993_RENDERPROFILE_INCLUDED
