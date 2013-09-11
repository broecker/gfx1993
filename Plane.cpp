#include "Plane.h"
#include "Line.h"
#include "glmHelper.h"

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

Plane::ClipResult Plane::clip(Line3D& line) const
{
	float da = distance(v3(line.a.position));
	float db = distance(v3(line.b.position));

	// both points outside of the plane
	if (da < 0.f && db < 0.f)
	{
		return BACK;
	}
	
	// both points inside
	if (da > 0.f && db > 0.f)
		return FRONT;
	
	// clipping, calculate clip point
	glm::vec4 v = line.b.position - line.a.position;
	float t = (-parameters.x*line.a.position.x -parameters.y*line.a.position.y - parameters.z*line.a.position.z - parameters.w) / (parameters.x*v.x + parameters.y*v.y + parameters.z*v.z);
		
	Vertex R = line.interpolate( t );
	
	if (da < 0.f)
		line.a = R;
	else
		line.b = R;
		
	return CLIP;

}

float Plane::distance(const glm::vec3& pt) const
{
	return (parameters.x*pt.x + parameters.y*pt.y + parameters.z*pt.z + parameters.w) / 
			sqrtf(parameters.x*parameters.x + parameters.y*parameters.y + parameters.z*parameters.z);
}