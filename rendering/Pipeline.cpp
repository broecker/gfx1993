#include "Pipeline.h"

#include <glm/glm.hpp>

using glm::vec4;

namespace render
{
    VertexOut&& lerp(const VertexOut& a, const VertexOut& b, float d)
    {
        VertexOut result;
        result.clipPosition = glm::mix(a.clipPosition, b.clipPosition, d);
        result.worldPosition = glm::mix(a.worldPosition, b.worldPosition, d);
        result.worldNormal = glm::mix(a.worldNormal, b.worldNormal, d);
        result.color = glm::mix(a.color, b.color, d);
        result.texcoord = glm::mix(a.texcoord, b.texcoord, d);
        return std::move(result);
    }

    ShadingGeometry&& interpolate(const ShadingGeometry& a, const ShadingGeometry& b, float d)
    {
        ShadingGeometry result;

        result.position = mix(a.position, b.position, d);
        result.normal = normalize(mix(a.normal, b.normal, d));

        result.color = mix(a.color, b.color, d);
        result.windowCoord = mix(a.windowCoord, b.windowCoord, d);

        return std::move(result);
    }


    // Access enum for the plane array
    enum ClippingPlane
    {
        LEFT = 0,
        RIGHT,
        BOTTOM,
        TOP,
        NEAR,
        FAR
    };

    // the six clipping planes in projective coordinates
    static const vec4 clippingPlanes[] = { vec4(1,0,0,1), vec4(-1,0,0,1), vec4(0,1,0,1), vec4(0,-1,0,1), vec4(0,0,1,1), vec4(0,0,-1,1) };


    // clips a line to a single clip space plane
    static ClipResult clipLineToPlane(VertexOut& a, VertexOut& b, ClippingPlane plane)
    {
        // signed distances from the clipping planes to both line end points
        float d0 = dot(a.clipPosition, clippingPlanes[plane]);
        float d1 = dot(b.clipPosition, clippingPlanes[plane]);

        // both are outside -- discard
        if (d0 < 0.f && d1 < 0.f)
            return DISCARD;

        // both are inside -- no clipping needed
        if (d0 >= 0.f && d1 >= 0.f)
            return KEEP;

        // calculate intersection;
        float t = d0 / (d0 - d1);

        // a is inside, b outside
        if (d0 >= 0.f)
            b = lerp(a, b, t);
            // a is outside, b inside
        else
            a = lerp(a, b, t);

        return CLIPPED;
    }

    static ClipResult clipTriangleToPlane(TrianglePrimitive& in, ClippingPlane plane)
    {
        float d0 = dot(in.a.clipPosition, clippingPlanes[plane]);
        float d1 = dot(in.b.clipPosition, clippingPlanes[plane]);
        float d2 = dot(in.c.clipPosition, clippingPlanes[plane]);

        // all outside
        if (d0 < 0.f && d1 < 0.f && d2 < 0.f)
            return DISCARD;

        // all inside
        if (d0 >= 0.f && d1 >= 0.f && d2 >= 0.f)
            return KEEP;

        // calculate intersection
        clipLineToPlane(in.a, in.b, plane);
        clipLineToPlane(in.a, in.c, plane);

        // TODO: create second triangle here.

        // TODO: Cull back faces here.

        return CLIPPED;
    }

    ClipResult PointPrimitive::clipToNDC() const
    {
        if (p.clipPosition.x >= -p.clipPosition.w && p.clipPosition.x <= p.clipPosition.w &&
            p.clipPosition.y >= -p.clipPosition.w && p.clipPosition.y <= p.clipPosition.w &&
            p.clipPosition.z >= -p.clipPosition.w && p.clipPosition.z <= p.clipPosition.w)
            return KEEP;
        else
            return DISCARD;
    }

    ShadingGeometry&& PointPrimitive::rasterize() const
    {
        ShadingGeometry result;
        result.position = p.worldPosition;
        result.normal = p.worldNormal;
        result.color = p.color;
        result.texcoord = p.texcoord;
        return std::move(result);
    }

    ClipResult LinePrimitive::clipToNDC()
    {
        ClipResult cr = KEEP;

        if ((cr = clipLineToPlane(a, b, NEAR)) == DISCARD)
            return DISCARD;
        if ((cr = clipLineToPlane(a, b, FAR)) == DISCARD)
            return DISCARD;
        if ((cr = clipLineToPlane(a, b, LEFT)) == DISCARD)
            return DISCARD;
        if ((cr = clipLineToPlane(a, b, RIGHT)) == DISCARD)
            return DISCARD;
        if ((cr = clipLineToPlane(a, b, TOP)) == DISCARD)
            return DISCARD;
        if ((cr = clipLineToPlane(a, b, BOTTOM)) == DISCARD)
            return DISCARD;

        return cr;
    }

    ShadingGeometry&& LinePrimitive::rasterize(float d) const
    {
        ShadingGeometry result;
        result.position = mix(a.worldPosition, b.worldPosition, d);
        result.normal = normalize(mix(a.worldNormal, b.worldNormal, d));
        result.color = mix(a.color, b.color, d);
        result.texcoord = mix(a.texcoord, b.texcoord, d);
        return std::move(result);
    }


// creates new triangles
    ClipResult TrianglePrimitive::clipToNDC()
    {
        ClipResult cr = KEEP;

        if ((cr = clipTriangleToPlane(*this, NEAR))   == DISCARD)
            return DISCARD;
        if ((cr = clipTriangleToPlane(*this, FAR))    == DISCARD)
            return DISCARD;
        if ((cr = clipTriangleToPlane(*this, LEFT))   == DISCARD)
            return DISCARD;
        if ((cr = clipTriangleToPlane(*this, RIGHT))  == DISCARD)
            return DISCARD;
        if ((cr = clipTriangleToPlane(*this, TOP))    == DISCARD)
            return DISCARD;
        if ((cr = clipTriangleToPlane(*this, BOTTOM)) == DISCARD)
            return DISCARD;

        return cr;

    }

    ShadingGeometry&& TrianglePrimitive::rasterize(const glm::vec3& bary) const
    {
        float bsum = bary.x+bary.y+bary.z;

        ShadingGeometry sgeo;
        sgeo.position = a.worldPosition*bary.x + b.worldPosition*bary.y + c.worldPosition*bary.z / bsum;
        sgeo.normal = a.worldNormal*bary.x + b.worldNormal*bary.y + c.worldNormal*bary.z / bsum;
        sgeo.color = a.color * bary.x + b.color * bary.y + c.color * bary.z / bsum;
        sgeo.texcoord = a.texcoord*bary.x + b.texcoord*bary.y + c.texcoord*bary.z / bsum;

        return std::move(sgeo);
    }


}


