#ifndef CAMERA_INCLUDED
#define CAMERA_INCLUDED

#include <glm/glm.hpp>

class Camera
{
public:
    virtual ~Camera() = default;

    virtual void handleKeyPress(unsigned char key) = 0;

    virtual void handleMousePress(int button, int state) = 0;

    virtual void handleMouseMove(const glm::ivec2 &delta) = 0;

    virtual glm::mat4 getViewMatrix() = 0;

    virtual glm::mat4 getProjectionMatrix() = 0;

};


class OrbitCamera : public Camera
{
public:
    OrbitCamera(const glm::vec3 &target, const glm::vec3 &up, float radius);

    void handleKeyPress(unsigned char key) override;

    void handleMousePress(int button, int state) override;

    void handleMouseMove(const glm::ivec2 &delta) override;

    glm::mat4 getViewMatrix() override;

    glm::mat4 getProjectionMatrix() override;

private:
    glm::mat4 projectionMatrix;

    glm::vec3 target, up, position;
    float radius;
    float phi, theta;

    enum MovementMode
    {
        ROTATE,
        PAN
    } mode;

    void updatePosition();
};


#endif
