#ifndef GFX1993_RENDERPROFILE_INCLUDED
#define GFX1993_RENDERPROFILE_INCLUDED

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace gfx1993 {

struct Stat {
  std::vector<uint32_t> samples;

  void add(uint32_t duration) { samples.push_back(duration); }
};

class RenderProfile;

// A RAII marker.
class ProfileMarker {
public:
  ~ProfileMarker();

private:
  friend class RenderProfile;
  ProfileMarker(const std::string& name, RenderProfile* owner);

  std::string name;
  uint32_t    startTime;

  // The profile that created this marker. We'll register the
  // time on destruction with this one.
  RenderProfile* owner;
};

struct RenderProfile {
  std::map<std::string, Stat> stats;

  void reset() { stats.clear(); }

  inline ProfileMarker startTiming(const std::string& name) {
    return ProfileMarker(name, this);
  }
  void endTiming(const ProfileMarker& marker); 

  void print() const;
};

}  // namespace gfx1993

#endif // GFX1993_RENDERPROFILE_INCLUDED
