#include "Camera.h"

#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <glm/ext.hpp>

using glm::cross;
using glm::dot;
using glm::normalize;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

namespace gfx1993 {
namespace util {

// To avoid gimbal lock.
constexpr float MIN_PITCH = -88.f;
constexpr float MAX_PITCH = 88.f;

static const mat4 defaultProjectionMatrix = glm::perspective(90.f, 1.3f, 1.f, 100.f);

Camera::Camera(const glm::mat4& projectionMatrix, const glm::vec3& position) :
  projectionMatrix(projectionMatrix), position(position), up(vec3(0,1,0)) {}

OrbitCamera::OrbitCamera(const glm::mat4& projectionMatrix, const glm::vec3& target, float radius) :
  Camera(projectionMatrix, position),
  target(target), radius(radius), phi(0), theta(0), mode(ROTATE) {}

OrbitCamera::OrbitCamera(const vec3 &t, float r) :
    Camera(defaultProjectionMatrix, position), target(t), radius(r), phi(0.f), theta(0.f), mode(ROTATE) {}

void OrbitCamera::handleInputTranslate(const vec3& delta) {
  radius += delta.z;
  radius = glm::max(1.f, radius -= 0.5f);

  updatePosition();
}

void OrbitCamera::handleInputRotate(const vec3 &delta) {
  if (mode == ROTATE) {
    phi += delta.x;
    theta += delta.y;
    phi = glm::clamp(phi, MIN_PITCH, MAX_PITCH);
    updatePosition();    
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

glm::mat4 OrbitCamera::getViewMatrix() const {
  return glm::lookAt(position, target, up);
}

void OrbitCamera::updatePosition() {
  position = glm::euclidean(glm::radians(glm::vec2(phi, theta))) * radius;
  position += target;
  // std::cout << "Camera: phi: " << phi << " theta: " << theta << " delta: (" << delta.x << "," << delta.y << ") position: (" << position.x << "," << position.y << "," << position.z << ")\n";
}

FreeCamera::FreeCamera(const glm::vec3& position) : 
  Camera(defaultProjectionMatrix, position), yaw(0), pitch(0),
    movementSpeed(1), rotationSpeed(20), velocity(0.f), maxSpeed(250.f), speedDecay(0.5f) {};

FreeCamera::FreeCamera(const glm::mat4& projectionMatrix, const glm::vec3& position) : 
  Camera(projectionMatrix, position), yaw(0), pitch(0),
    movementSpeed(1), rotationSpeed(20), velocity(0.f), maxSpeed(250.f), speedDecay(0.5f) {};


void FreeCamera::handleInputTranslate(const vec3& delta) {
  const vec3 right = getRight();

  velocity += forward * delta.z * movementSpeed.z;
  velocity += right * delta.x * movementSpeed.x;
  velocity += up * delta.y * movementSpeed.y;
  
  // Safety-check to avoid division by zero.
  if (dot(velocity, velocity) > 0.1) {
    velocity = glm::normalize(velocity);
    velocity *= maxSpeed;
  } else {
    velocity = vec3(0);
  }
}

void FreeCamera::update(float dt) {
  position += velocity * dt;
  velocity *= speedDecay;
}

void FreeCamera::handleInputRotate(const vec3 &delta) {
  yaw -= delta.y * movementSpeed.y;
  pitch += delta.x * movementSpeed.x;

  pitch = glm::clamp(pitch, MIN_PITCH, MAX_PITCH);

  float yrad = glm::radians(yaw);
  float prad = glm::radians(pitch);

  // See https://www.mauriciopoppe.com/notes/computer-graphics/viewing/camera/first-person-shot/
  // It constructs a rotation matrix.
  forward.x = -sin(yrad) * cos(prad);
  forward.y = sin(prad);
  forward.z = -cos(yrad) * cos(prad);
}

glm::mat4 FreeCamera::getViewMatrix() const {
  return glm::lookAt(position, position + forward, up);
}

}  // namespace util
}  // namespace gfx1993