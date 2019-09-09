#include "Camera.h"

#include <glm/gtx/io.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <memory>

using glm::vec3;
using glm::vec4;

Camera::~Camera() 
{
}

OrbitCamera::OrbitCamera(const vec3& t, const vec3& u, float r) : target(t), up(u), position(t), radius(r), phi(0.f), theta(0.f)
{
}

void OrbitCamera::handleKeyPress(unsigned char key)
{
	switch(key) {
		case 'a':
		case '-':
			radius *= 1.4f;
			break;
		case 'z':
		case '=':
		case '+':
			radius /= 1.4f;
			break;
		default:
			break;
	}
}

void OrbitCamera::handleMouseMove(const glm::ivec2& delta)
{
	phi += delta.y;
	theta += delta.x;

	phi = glm::clamp(phi, -89.f, 89.f);
}

glm::mat4 OrbitCamera::getCameraMatrix()
{
	position = glm::euclidean(glm::radians(glm::vec2(phi, theta))) * radius;
	position += target;

	return glm::lookAt(position, target, up);
}