#ifndef PLANE_INCLUDED
#define PLANE_INCLUDED

#include <glm/glm.hpp>

struct Line3D;

struct Plane
{
	glm::vec4	parameters;
	
	enum ClipResult
	{
		FRONT,
		BACK,
		CLIP
	};

	/// Create a plane from three points
	Plane();
	Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	void set(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

	ClipResult clip(Line3D& line) const;


	float distance(const glm::vec3& pt) const;

};

#endif