#include <GUnit.h>

#include <limits>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "Camera.h"

namespace gfx1993 {

using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

inline bool equalMat4(const mat4 &a, const mat4 &b) {
  const float eps = 1e-4f;
  return glm::all(glm::epsilonEqual(a[0], b[0], eps)) &&
        glm::all(glm::epsilonEqual(a[1], b[1], eps)) &&
        glm::all(glm::epsilonEqual(a[2], b[2], eps)) &&
        glm::all(glm::epsilonEqual(a[3], b[3], eps));
}

GTEST("Camera Test") {
  SHOULD("OrbitCamera(target, radius) has a valid position immediately at "
        "construction, before any input") {
    // Regression test: OrbitCamera's constructor used to seed the base
    // Camera's position from its own not-yet-constructed `position` member
    // (garbage), and never called updatePosition() until the first
    // handleInputRotate() -- i.e. the first mouse drag. Before that, the
    // camera pointed at an undefined position and demos rendered a blank
    // screen.
    const vec3 target(0, 0, 0);
    const float radius = 10.f;
    OrbitCamera camera(target, radius);

    // phi = theta = 0 by construction; matches OrbitCamera::updatePosition().
    const vec3 expectedPosition =
        glm::euclidean(glm::radians(vec2(0.f, 0.f))) * radius + target;
    const mat4 expectedView =
        glm::lookAt(expectedPosition, target, vec3(0, 1, 0));

    EXPECT(equalMat4(camera.getViewMatrix(), expectedView));
  }

  SHOULD("OrbitCamera(projectionMatrix, target, radius) has a valid position "
        "immediately at construction, before any input") {
    const mat4 projection = glm::perspective(45.f, 1.f, 1.f, 100.f);
    const vec3 target(1, 2, 3);
    const float radius = 5.f;
    OrbitCamera camera(projection, target, radius);

    const vec3 expectedPosition =
        glm::euclidean(glm::radians(vec2(0.f, 0.f))) * radius + target;
    const mat4 expectedView =
        glm::lookAt(expectedPosition, target, vec3(0, 1, 0));

    EXPECT(equalMat4(camera.getViewMatrix(), expectedView));
  }

  SHOULD("FreeCamera(projectionMatrix, position) has a valid forward vector "
        "immediately at construction, before any input") {
    // Regression test: FreeCamera's `forward` member was never initialized
    // in either constructor, so getViewMatrix()'s `position + forward` look
    // target was garbage until the first handleInputRotate() call computed
    // a real forward vector from yaw/pitch.
    const mat4 projection = glm::perspective(45.f, 1.f, 1.f, 100.f);
    const vec3 position(5, 0, 0);
    FreeCamera camera(projection, position);

    // yaw = pitch = 0 by construction; matches FreeCamera::updateForward().
    const vec3 expectedForward(0, 0, -1);
    const mat4 expectedView =
        glm::lookAt(position, position + expectedForward, vec3(0, 1, 0));

    EXPECT(equalMat4(camera.getViewMatrix(), expectedView));
  }

  SHOULD("FreeCamera(position) has a valid forward vector immediately at "
        "construction, before any input") {
    const vec3 position(0, 0, 0);
    FreeCamera camera(position);

    // yaw = 0, pitch = 90 by construction; matches FreeCamera::updateForward().
    const float prad = glm::radians(90.f);
    const vec3 expectedForward(-sin(0.f) * cos(prad), sin(prad), -cos(0.f) * cos(prad));
    const mat4 expectedView =
        glm::lookAt(position, position + expectedForward, vec3(0, 1, 0));

    EXPECT(equalMat4(camera.getViewMatrix(), expectedView));
  }
}

}  // namespace gfx1993
