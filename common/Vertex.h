#ifndef VERTEX_INCLUDED
#define VERTEX_INCLUDED

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec4 	position;
	glm::vec3	normal;
	glm::vec4 	colour;
	glm::vec2	texcoord; 

	inline Vertex() : position(0,0,0,1), normal(0,0,0), colour(1,1,1,1), texcoord(0,0) {}
	inline Vertex(const glm::vec4& pos) : position(pos), normal(0,0,0), colour(0,0,0,1), texcoord(0,0) {}
	inline Vertex(const glm::vec4& pos, const glm::vec3& n, const glm::vec4& col, const glm::vec2& tc) : 
		position(pos), normal(n), colour(col), texcoord(tc) {}
	
	inline const Vertex& operator = (const Vertex& cp)
	{
		position = cp.position;
		normal = cp.normal;
		colour = cp.colour;
		texcoord = cp.texcoord;
		
		return *this;
	}
	
};

// output of a vertex shader
struct VertexOut
{
	// after view and projection transform
	glm::vec4	clipPosition;

	// for shading -- position and normal in world coords
	glm::vec3	worldPosition;
	glm::vec3	worldNormal;
	
	// material info here
	glm::vec4	colour;
	glm::vec2	texcoord;
};

inline VertexOut lerp(const VertexOut& a, const VertexOut& b, float d)
{
	VertexOut result;
	result.clipPosition = glm::mix(a.clipPosition, b.clipPosition, d);
	result.worldPosition = glm::mix(a.worldPosition, b.worldPosition, d);
	result.worldNormal = glm::mix(a.worldNormal, b.worldNormal, d);
	result.colour = glm::mix(a.colour, b.colour, d);
	result.texcoord = glm::mix(a.texcoord, b.texcoord, d);
	return result;
}



typedef std::vector<Vertex> VertexList;
typedef std::vector<unsigned int> IndexList;
typedef std::vector<VertexOut> VertexOutList;


#endif