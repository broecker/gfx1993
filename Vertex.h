#ifndef VERTEX_INCLUDED
#define VERTEX_INCLUDED

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec4 	position;
	glm::vec4 	colour; 

	inline Vertex() : position(0,0,0,1), colour(1,1,1,1) {}
	inline Vertex(const glm::vec4& pos, const glm::vec4& col) : position(pos), colour(col) {}
	
	inline const Vertex& operator = (const Vertex& cp)
	{
		position = cp.position;
		colour = cp.colour;
		
		return *this;
	}
	
};

typedef std::vector<Vertex> VertexList;
typedef std::vector<unsigned int> IndexList;

#endif