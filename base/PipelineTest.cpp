#include <GUnit.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Pipeline.h"

namespace gfx1993 {
namespace render {

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;

GTEST("Pipeline Test") {
  SHOULD("Rasterize point") {
    VertexOut vertexOut;
    vertexOut.clipPosition = vec4(0,0,0,1);
    vertexOut.worldPosition = vec3(0);
    vertexOut.worldNormal = vec3(0);
    vertexOut.color = vec4(1,0,0,1);
    vertexOut.texcoord = vec2(0,1);

    vertexOut.varying[0] = vec4(0,0,0,1);
    vertexOut.varying[1] = vec4(0,0,1,0);
    vertexOut.varying[2] = vec4(0,0,1,1);
    vertexOut.varying[3] = vec4(0,1,0,0);

    PointPrimitive point(vertexOut);

    ShadingGeometry sgeo = point.rasterize();

    EXPECT(sgeo.position == vertexOut.worldPosition);
    EXPECT(sgeo.normal == vertexOut.worldNormal);
    EXPECT(sgeo.color == vertexOut.color);
    EXPECT(sgeo.texcoord == vertexOut.texcoord);
    EXPECT(sgeo.varying[0] == sgeo.varying[0]);
    EXPECT(sgeo.varying[1] == sgeo.varying[1]);
    EXPECT(sgeo.varying[2] == sgeo.varying[2]);
    EXPECT(sgeo.varying[3] == sgeo.varying[3]);

    // WindowCoord and depth are filled in by the
    // rasterizer during perspective division.
  }

  SHOULD("Rasterize lines") {
    VertexOut a;
    a.clipPosition = vec4(0,0,0,1);
    a.worldPosition = vec3(0);
    a.worldNormal = vec3(0);
    a.color = vec4(1,0,0,1);
    a.texcoord = vec2(0,1);

    a.varying[0] = vec4(0,0,0,1);
    a.varying[1] = vec4(0,0,1,0);
    a.varying[2] = vec4(0,0,1,1);
    a.varying[3] = vec4(0,1,0,0);

    VertexOut b;
    b.clipPosition = vec4(2,0,0,1);
    b.worldPosition = vec3(2,0,0);
    b.worldNormal = vec3(0);
    b.color = vec4(0,0,1,1);
    b.texcoord = vec2(0,0);
    b.varying[0] = vec4(0,0,0,1);
    b.varying[1] = vec4(0,0,1,0);
    b.varying[2] = vec4(0,0,1,1);
    b.varying[3] = vec4(0,1,0,0);

    LinePrimitive line(a, b);

    ShadingGeometry endA = line.rasterize(0);
    EXPECT(endA.position == a.worldPosition);
    
    ShadingGeometry endB = line.rasterize(1);
    EXPECT(endB.position == b.worldPosition);

    ShadingGeometry middle = line.rasterize(0.5);
    EXPECT(middle.position == vec3(1,0,0));
    EXPECT(middle.color == vec4(0.5, 0, 0.5, 1));
    EXPECT(middle.texcoord == vec2(0, 0.5));
  }

  SHOULD("Rasterize triangles") {
    VertexOut a;
    a.clipPosition = vec4(0,0,0,1);
    a.worldPosition = vec3(0);
    a.worldNormal = vec3(0);
    a.color = vec4(1,0,0,1);
    a.texcoord = vec2(0,1);

    a.varying[0] = vec4(0,0,0,1);
    a.varying[1] = vec4(0,0,1,0);
    a.varying[2] = vec4(0,0,1,1);
    a.varying[3] = vec4(0,1,0,0);

    VertexOut b;
    b.clipPosition = vec4(0,0,0,1);
    b.worldPosition = vec3(0);
    b.worldNormal = vec3(0);
    b.color = vec4(0,1,0,1);
    b.texcoord = vec2(0,1);

    b.varying[0] = vec4(0,0,0,1);
    b.varying[1] = vec4(0,0,1,0);
    b.varying[2] = vec4(0,0,1,1);
    b.varying[3] = vec4(0,1,0,0);

    VertexOut c;
    c.clipPosition = vec4(0,0,0,1);
    c.worldPosition = vec3(0);
    c.worldNormal = vec3(0);
    c.color = vec4(0,0,1,1);
    c.texcoord = vec2(0,1);

    c.varying[0] = vec4(0,0,0,1);
    c.varying[1] = vec4(0,0,1,0);
    c.varying[2] = vec4(0,0,1,1);
    c.varying[3] = vec4(0,1,0,0);

    TrianglePrimitive tri(a,b,c);

    ShadingGeometry geoPointA = tri.rasterize(vec3(1,0,0));
    EXPECT(geoPointA.position == a.worldPosition);

    ShadingGeometry geoPointB = tri.rasterize(vec3(0,1,0));
    EXPECT(geoPointB.position == b.worldPosition);

    ShadingGeometry geoPointC = tri.rasterize(vec3(0,0,1));
    EXPECT(geoPointC.position == c.worldPosition);

    ShadingGeometry center = tri.rasterize(vec3(0.5));
    EXPECT(center.position == (a.worldPosition + b.worldPosition + c.worldPosition) / vec3(3.f));
    EXPECT(center.color == vec4(1.f/3, 1.f/3, 1.f/3, 1.f));
  }

  SHOULD("Interpolate shading geometries") {
    ShadingGeometry a;
    a.position = vec3(1,0,0);
    a.color = vec4(1,0,0,1);
    a.texcoord = vec2(0,0);
    a.depth = 0.25;

    ShadingGeometry b;
    b.position = vec3(0,1,0);
    b.color = vec4(1,0,0,1);
    b.texcoord = vec2(1,0);
    b.depth = 0.75;

    ShadingGeometry c = interpolate(a, b, 0.5);

    EXPECT(c.position == vec3(0.5, 0.5, 0));
    EXPECT(c.color == vec4(1,0,0,1));
    EXPECT(c.texcoord == vec2(0.5, 0));
    EXPECT(c.depth == 0.5);
  }
};

}  // namespace render
}  // namespace gfx1993