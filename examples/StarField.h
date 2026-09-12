#ifndef GFX1993_STARFIELD_INCLUDED
#define GFX1993_STARFIELD_INCLUDED

#include "Geometry.h"

#include <string>
#include <vector>

namespace gfx1993 {

// A real starfield, loaded from a small catalog of the brightest stars
// (right ascension, declination, apparent magnitude, B-V color index).
//
// Unlike other Geometry subclasses, the visible point set changes over time
// (Earth's rotation) and isn't recomputed every frame: call
// updateVisibleStars() periodically (e.g. every few frames) with the
// current sidereal angle and observer latitude to rebuild vertices/indices
// with only the currently above-horizon stars, as real world-space points.
class StarField : public Geometry {
public:
  // Loads a CSV catalog with columns "name,ra_hours,dec_deg,mag,ci" (lines
  // starting with '#' are treated as comments). Populates the internal
  // per-star catalog; does not populate vertices/indices -- call
  // updateVisibleStars() at least once after loading before drawing.
  bool loadCatalog(const std::string &filename);

  // Recomputes vertices/indices to contain only the stars currently above
  // the horizon (altitude > 0), as world-space points at the given
  // distance from the origin, using standard equatorial-to-horizontal
  // coordinate conversion:
  //   siderealAngle: current hour-angle reference, in radians (increases
  //     as time passes -- one full rotation per sidereal day).
  //   observerLatitudeRadians: observer's geographic latitude.
  //   distance: world-space distance to place each star point at (must be
  //     within the camera's projection far plane, or points get clipped).
  //   brightnessScale: multiplies each star's precomputed base color, e.g.
  //     to fade the whole field in/out with the sun's elevation. 0 skips
  //     the coordinate math entirely and clears the field.
  void updateVisibleStars(float siderealAngle, float observerLatitudeRadians,
                          float distance, float brightnessScale);

private:
  struct CatalogStar {
    float raRadians;
    float decRadians;
    glm::vec3 baseColor; // tint * normalized brightness, precomputed at load time
  };

  std::vector<CatalogStar> catalog;
};

} // namespace gfx1993

#endif
