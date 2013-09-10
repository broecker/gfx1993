#include "Frustum.h"

static inline glm::vec3 v3(const glm::vec4& v) 
{
	return glm::vec3(v.x, v.y, v.z);
}

void Frustum::calculate(const glm::mat4& mvp)
{
	using namespace glm;

	// calculate the eight frustum vertices in world coords
	static const glm::vec4 frustumVerticesNDC[8] = {glm::vec4(-1,-1,-1,1), 
													glm::vec4(-1, 1,-1,1),
													glm::vec4( 1, 1,-1,1),
													glm::vec4( 1,-1,-1,1),
													glm::vec4(-1,-1, 1,1),
													glm::vec4(-1, 1, 1,1),
													glm::vec4( 1, 1, 1,1),
													glm::vec4( 1,-1, 1,1) };

	glm::mat4 imvp = glm::inverse( mvp );

	glm::vec4 frustumVerticesWorld[8];
	for (int i = 0; i < 8; ++i)
	{
		frustumVerticesWorld[i] = imvp * frustumVerticesNDC[i];
		frustumVerticesWorld[i] / frustumVerticesWorld[i].w;
	}

	// set the six frustum planes
	near.set(   v3(frustumVerticesWorld[0]), v3(frustumVerticesWorld[1]), v3(frustumVerticesWorld[3]) );
	far.set(    v3(frustumVerticesWorld[7]), v3(frustumVerticesWorld[6]), v3(frustumVerticesWorld[4]) );
	left.set(   v3(frustumVerticesWorld[4]), v3(frustumVerticesWorld[5]), v3(frustumVerticesWorld[0]) );
	right.set(  v3(frustumVerticesWorld[3]), v3(frustumVerticesWorld[2]), v3(frustumVerticesWorld[7]) );
	top.set(    v3(frustumVerticesWorld[1]), v3(frustumVerticesWorld[5]), v3(frustumVerticesWorld[2]) );
	bottom.set( v3(frustumVerticesWorld[4]), v3(frustumVerticesWorld[0]), v3(frustumVerticesWorld[7]) );
}