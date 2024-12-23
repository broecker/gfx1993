#ifndef CAMERA_INCLUDED
#define CAMERA_INCLUDED

#include <glm/glm.hpp>

namespace gfx1993 {
namespace util {

class Camera {
public:
  virtual ~Camera() = default;

  virtual void handleKeyPress(unsigned char key) = 0;

  virtual void handleMousePress(int button, int state) = 0;

  virtual void handleMouseMove(const glm::ivec2 &delta) = 0;

  virtual glm::mat4 getViewMatrix() const = 0;

  virtual glm::mat4 getProjectionMatrix() const = 0;
};

class OrbitCamera : public Camera {
public:
  OrbitCamera(const glm::vec3 &target, const glm::vec3 &up, float radius);

  void handleKeyPress(unsigned char key) override;

  void handleMousePress(int button, int state) override;

  void handleMouseMove(const glm::ivec2 &delta) override;

  glm::mat4 getViewMatrix() const override;

  glm::mat4 getProjectionMatrix() const override;

  inline void setTarget(const glm::vec3& tgt) { target = tgt; }

private:
  glm::mat4 projectionMatrix;

  glm::vec3 target, up, position;
  float radius;
  float phi, theta;

  enum MovementMode { ROTATE, PAN } mode;

  void updatePosition();
};

}  // namespace util
}  // namespace gfx1993

#endif
