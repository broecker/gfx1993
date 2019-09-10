#ifndef FRAGMENT_INCLUDED
#define FRAGMENT_INCLUDED

#include <glm/glm.hpp>

namespace render 
{

struct Fragment
{
	glm::ivec3 		coords;
	glm::vec4		colour;
};

}

#endif