#include "Geometry.h"

#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cassert>
#include <iostream>

static inline void scaleNormal(Vertex& v)
{
	glm::normalize( v.normal );
}

bool Geometry::loadPLY(const std::string& filename)
{
	vertices.clear();
	indices.clear();

	boundingSphereRadius = 0.f;

	std::ifstream file(filename.c_str());
	if (!file.is_open())
	{
		std::cerr << "Unable to open file \"" << filename << "\"\n";
		return false;
	}
	else
		std::clog << "Loading file " << filename << std::endl;

	std::string buffer;

	bool readHeader = true;
	unsigned int vertexCount = 0, readVertices = 0;
	unsigned int faceCount = 0, readFaces = 0;
	while (!file.eof())
	{
		std::getline(file, buffer);

		if (buffer.empty())
			continue;

		if (readHeader)
		{
			if (buffer == "end_header")
				readHeader = false;

			if (buffer.find("element vertex") != std::string::npos)
			{
				assert( sscanf(buffer.c_str(), "element vertex %i", &vertexCount) == 1 );
				vertices.reserve( vertexCount );
			}

			if (buffer.find("element face") != std::string::npos)
			{
				assert( sscanf(buffer.c_str(), "element face %i", &faceCount) == 1);
				indices.reserve( faceCount*3 );
			}

			continue;
		}

		// read number of vertices
		if (readVertices < vertexCount)
		{
			float x, y, z, c, i;
			assert( sscanf(buffer.c_str(), "%f %f %f %f %f", &x, &y, &z, &c, &i) == 5);

			vertices.push_back( Vertex( glm::vec4(x,y,z,1) ) );
		
			boundingSphereRadius = std::max(	boundingSphereRadius,
												sqrtf(x*x+y*y+z*z) );

			++readVertices;
		}

		// and then the faces


		else
		{
			unsigned int a, b, c;
			assert( sscanf(buffer.c_str(), "3 %i %i %i", &a, &b, &c) == 3);

			indices.push_back( a );
			indices.push_back( b );
			indices.push_back( c );
		
			if (a >= vertices.size())
				std::cerr << "Illegal index: " << a << "?\n";
		
			if (b >= vertices.size())
				std::cerr << "Illegal index: " << b << "?\n";

			if (c >= vertices.size())
				std::cerr << "Illegal index: " << c << "?\n";
		}


	}

	std::clog << "Read " << vertices.size() << " vertices, " << indices.size() << " indices\n";

	// calculate normals
	for (unsigned int i = 0; i < indices.size(); i += 3)
	{

		// triangle
		Vertex& a = vertices[ indices[i+0] ];
		Vertex& b = vertices[ indices[i+1] ];
		Vertex& c = vertices[ indices[i+2] ];

		// normal
		glm::vec3 n = glm::normalize( glm::cross(	glm::vec3(b.position-a.position), 
													glm::vec3(c.position-a.position) ) );

		a.normal += n;
		b.normal += n;
		c.normal += n;
	}


	// scale normal to length 1;
	std::for_each(vertices.begin(), vertices.end(), scaleNormal);



}