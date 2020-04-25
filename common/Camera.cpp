#include "Camera.h"

#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/transform.hpp>

using glm::cross;
using glm::dot;
using glm::normalize;
using glm::vec2;
using glm::vec3;
using glm::vec4;

OrbitCamera::OrbitCamera(const vec3 &t, const vec3 &u, float r)
    : projectionMatrix(glm::perspective(90.f, 1.3f, 1.f, 100.f)), target(t),
      up(u), position(t), radius(r), phi(0.f), theta(0.f), mode(ROTATE) {}

void OrbitCamera::handleKeyPress(unsigned char key) {
  switch (key) {
  case 'a':
  case '-':
    // radius *= 1.4f;
    radius += 0.4f;
    break;
  case 'z':
  case '=':
  case '+':
    // Don't go closer than the near plane.
    radius = glm::max(1.f, radius -= 0.5f);
    // radius -= 0.4f;
    break;
  default:
    break;
  }
}

void OrbitCamera::handleMousePress(int button, int state) {
  if (button == 1 && state == 0) {
    mode = PAN;
  }

  if (button == 1 && state == 1) {
    mode = ROTATE;
  }
}

void OrbitCamera::handleMouseMove(const glm::ivec2 &delta) {
  if (mode == ROTATE) {
    phi += delta.y;
    theta += delta.x;
    phi = glm::clamp(phi, -89.f, 89.f);
  }

  if (mode == PAN) {
    // Calculate 3D vector from up and right;
    vec3 forward = normalize(target - position);
    vec3 right = cross(forward, up);
    vec3 realUp = cross(right, forward);

    vec2 movementScale = vec2(-0.2f, 0.2f) * vec2(delta);

    vec3 positionDelta = right * movementScale.x + realUp * movementScale.y;

    target += positionDelta;
    position += positionDelta;
  }
}

glm::mat4 OrbitCamera::getViewMatrix() {
  position = glm::euclidean(glm::radians(glm::vec2(phi, theta))) * radius;
  position += target;

  return glm::lookAt(position, target, up);
}

glm::mat4 OrbitCamera::getProjectionMatrix() { return projectionMatrix; }