#ifndef CAMERA_INCLUDED
#define CAMERA_INCLUDED

#include <glm/glm.hpp>

class Camera 
{
public:
	virtual ~Camera();

	virtual void handleKeyPress(unsigned char key) = 0;
	virtual void handleMouseMove(const glm::ivec2& delta) = 0;

	virtual glm::mat4 getCameraMatrix() = 0;
};


class OrbitCamera : public Camera 
{
public:
	OrbitCamera(const glm::vec3& target, const glm::vec3& up, float radius);

	void handleKeyPress(unsigned char key);
	void handleMouseMove(const glm::ivec2& delta);

	glm::mat4 getCameraMatrix();

private:
	glm::vec3		target, up, position;
	float 			radius;
	float			phi, theta;

	void updatePosition();
};



#endif
