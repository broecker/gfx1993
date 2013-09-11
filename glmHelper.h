//
//  glmHelper.h
//  srenderer
//
//  Created by Markus Broecker on 11/09/13.
//  Copyright (c) 2013 Markus Broecker. All rights reserved.
//

#ifndef srenderer_glmHelper_h
#define srenderer_glmHelper_h

#include <iostream>


inline std::ostream& operator << (std::ostream& os, const glm::vec4& v)
{
	return os << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
}
	
inline std::ostream& operator << (std::ostream& os, const glm::vec3& v)
{
	return os << "(" << v.x << "," << v.y << "," << v.z << ")";
}

inline std::ostream& operator << (std::ostream& os, const glm::mat4& m)
{
	os << "|" << m[0].x << "\t" << m[1].x << "\t" << m[2].x << "\t" << m[3].x << "|" << std::endl;
	os << "|" << m[0].y << "\t" << m[1].y << "\t" << m[2].y << "\t" << m[3].y << "|" << std::endl;
	os << "|" << m[0].z << "\t" << m[1].z << "\t" << m[2].z << "\t" << m[3].z << "|" << std::endl;
	os << "|" << m[0].w << "\t" << m[1].w << "\t" << m[2].w << "\t" << m[3].w << "|" << std::endl;
	
	return os;
}


#endif
