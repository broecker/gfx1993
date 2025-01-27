#ifndef CAMERA_INCLUDED
#define CAMERA_INCLUDED

#include <glm/glm.hpp>

namespace gfx1993 {

class Camera {
public:
  Camera(const glm::mat4& projectionMatrix, const glm::vec3& position);
  virtual ~Camera() = default;


  // Handles input rotation relative to the current camera orientation.
  // The vec3 corresponds to the camera's axis: X=pitch, Y=yaw, Z=roll.
  // This input usually comes from mouse movement.
  virtual void handleInputRotate(const glm::vec3& delta) = 0;

  // Handles input translations relative to the current camera orientation.
  // The vec3 corresponds to the delta movement along the camera's axis.
  // This input often comes from keyboard commands, for examples WASD in FPS
  // games.
  virtual void handleInputTranslate(const glm::vec3& delta) = 0;

  virtual glm::mat4 getViewMatrix() const = 0;

  virtual const glm::mat4& getProjectionMatrix() const {
    return projectionMatrix;
  }

  // Called once per frame before rendering.
  virtual void update(float dt) {};

protected:
  glm::mat4 projectionMatrix;

  glm::vec3 position, up;
};

// A camera that rotates around a fixed point.
class OrbitCamera : public Camera {
public:
  OrbitCamera(const glm::mat4& projectionMatrix, const glm::vec3& target, float radius);
  OrbitCamera(const glm::vec3 &target, float radius);

  void handleInputRotate(const glm::vec3& delta) override;

  void handleInputTranslate(const glm::vec3& delta) override;

  glm::mat4 getViewMatrix() const override;

  inline void setTarget(const glm::vec3& tgt) { target = tgt; }

private:
  glm::vec3 target;
  float radius;
  float phi, theta;

  enum MovementMode { ROTATE, PAN } mode;

  void updatePosition();
};

class FreeCamera : public Camera {
public:
  FreeCamera(const glm::mat4& projectionMatrix, const glm::vec3& position);
  FreeCamera(const glm::vec3& position);

  void handleInputRotate(const glm::vec3& delta) override;

  void handleInputTranslate(const glm::vec3& delta) override;

  glm::mat4 getViewMatrix() const override;

  void update(float dt) override;

private:
  float yaw, pitch;

  glm::vec3 movementSpeed;
  glm::vec2 rotationSpeed;

  glm::vec3 forward;

  glm::vec3 velocity;
  float maxSpeed;
  float speedDecay;


  inline glm::vec3 getRight() const {
    return glm::normalize(glm::cross(forward, up));
  }
};

}  // namespace gfx1993

#endif
