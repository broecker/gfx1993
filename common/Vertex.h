#ifndef VERTEX_INCLUDED
#define VERTEX_INCLUDED

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec4 	position;
	glm::vec3	normal;
	glm::vec4 	color;
	glm::vec2	texcoord; 

	inline Vertex() : position(0,0,0,1), normal(0,0,0), color(1, 1, 1, 1), texcoord(0, 0) {}
	inline explicit Vertex(const glm::vec4& pos) : position(pos), normal(0,0,0), color(0, 0, 0, 1), texcoord(0, 0) {}
	inline Vertex(const glm::vec4& pos, const glm::vec3& n, const glm::vec4& col, const glm::vec2& tc) :
            position(pos), normal(n), color(col), texcoord(tc) {}
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
	glm::vec4	color;
	glm::vec2	texcoord;
};

inline VertexOut&& lerp(const VertexOut& a, const VertexOut& b, float d)
{
	VertexOut result;
	result.clipPosition = glm::mix(a.clipPosition, b.clipPosition, d);
	result.worldPosition = glm::mix(a.worldPosition, b.worldPosition, d);
	result.worldNormal = glm::mix(a.worldNormal, b.worldNormal, d);
	result.color = glm::mix(a.color, b.color, d);
	result.texcoord = glm::mix(a.texcoord, b.texcoord, d);
	return std::move(result);
}



typedef std::vector<Vertex> VertexList;
typedef std::vector<unsigned int> IndexList;
typedef std::vector<VertexOut> VertexOutList;


#endif