#ifndef PLANE_INCLUDED
#define PLANE_INCLUDED

#include <glm/glm.hpp>

struct Line3D;

struct Plane
{
	glm::vec4	parameters;

	/// Create a plane from three points
	Plane();
	Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	void set(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	bool clip(const Line3D& in, Line3D& front, Line3D& back) const;


	float distance(const glm::vec3& pt) const;

};

#endif