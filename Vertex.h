#ifndef VERTEX_INCLUDED
#define VERTEX_INCLUDED

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec4 	position;
	glm::vec4 	colour; 

	inline Vertex(const glm::vec4& pos, const glm::vec4& col) : position(pos), colour(col) {}
};

typedef std::vector<Vertex> VertexList;
typedef std::vector<unsigned int> IndexList;

#endif