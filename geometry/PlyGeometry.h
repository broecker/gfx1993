#ifndef PLY_GEOMETRY_INCLUDED
#define PLY_GEOMETRY_INCLUDED

#include "Geometry.h"

#include <string>

namespace geo
{

class PlyGeometry : public Geometry
{
public:
	PlyGeometry();

	bool loadPly(const std::string& filename);

	// Centers the geometry without changing the transform.
	void center();

private:
	float		boundingSphereRadius;
};

}

#endif