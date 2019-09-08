#include "Camera.h"

#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

}

glm::mat4 OrbitCamera::getCameraMatrix()
{
	position = glm::euclidean(glm::radians(glm::vec2(theta, phi))) * radius;
	position += target;

	glm::mat4 result = glm::lookAt(position, target, up);

	return result * glm::scale(glm::vec3(1,1,-1));
}