#include "Plane.h"
#include "Line.h"

static inline glm::vec3 v3(const glm::vec4& v)
{
	return glm::vec3(v.x, v.y, v.z);
}

Plane::Plane() : parameters(0, 1, 0, 0)
{
}

Plane::Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	this->set(a, b, c);
}

void Plane::set(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{	
	using namespace glm;

	vec3 n = normalize(cross(b-a, c-a));
	float d = -n.x * a.x - n.y*a.y - n.z*a.z;

	parameters.x = n.x;
	parameters.y = n.y;
	parameters.z = n.z;
	parameters.w = d;
}

bool Plane::clip(const Line3D& in, Line3D& front, Line3D& back) const
{
	float da = distance(v3(in.a.position));
	float db = distance(v3(in.b.position));

	// both points outside of the plane
	if (da > 0.f && db > 0.f)
	{
		return false;
	}

}

float Plane::distance(const glm::vec3& pt) const
{
	return (parameters.x*pt.x + parameters.y*pt.y + parameters.z*pt.z + parameters.w) / 
			sqrtf(parameters.x*parameters.x + parameters.y*parameters.y + parameters.z*parameters.z);
}