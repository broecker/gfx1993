#ifndef VERTEX_INCLUDED
#define VERTEX_INCLUDED

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec4 	position;
	glm::vec3	normal;
	glm::vec4 	colour; 

	inline Vertex() : position(0,0,0,1), normal(0,0,0), colour(1,1,1,1) {}
	inline Vertex(const glm::vec4& pos, const glm::vec3& n, const glm::vec4& col) : position(pos), normal(n), colour(col) {}
	
	inline const Vertex& operator = (const Vertex& cp)
	{
		position = cp.position;
		normal = cp.normal;
		colour = cp.colour;
		
		return *this;
	}
	
};

typedef std::vector<Vertex> VertexList;
typedef std::vector<unsigned int> IndexList;

#endif