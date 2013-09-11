#include "Frustum.h"
#include "glmHelper.h"


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
	plane[NEAR].set(   v3(frustumVerticesWorld[0]), v3(frustumVerticesWorld[1]), v3(frustumVerticesWorld[3]) );
	plane[FAR].set(    v3(frustumVerticesWorld[7]), v3(frustumVerticesWorld[6]), v3(frustumVerticesWorld[4]) );
	plane[LEFT].set(   v3(frustumVerticesWorld[4]), v3(frustumVerticesWorld[5]), v3(frustumVerticesWorld[0]) );
	plane[RIGHT].set(  v3(frustumVerticesWorld[3]), v3(frustumVerticesWorld[2]), v3(frustumVerticesWorld[7]) );
	plane[TOP].set(    v3(frustumVerticesWorld[1]), v3(frustumVerticesWorld[5]), v3(frustumVerticesWorld[2]) );
	plane[BOTTOM].set( v3(frustumVerticesWorld[4]), v3(frustumVerticesWorld[0]), v3(frustumVerticesWorld[7]) );
}