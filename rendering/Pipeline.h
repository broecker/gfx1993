#ifndef SRENDER_PIPELINE_H
#define SRENDER_PIPELINE_H

#include <vector>
#include <glm/glm.hpp>

// Defines the high-level overview of the render pipeline.
// The full pipeline looks like this:
// Vertex -> [Vertex Shader] -> VertexOut -> RenderPrimitive -> ShadingGeometry -> [FragmentShader] -> Fragment
namespace render
{
    struct Vertex;
    struct VertexOut;
    struct PointPrimitive;
    struct LinePrimitive;
    struct TrianglePrimitive;
    struct ShadingGeometry;
    struct Fragment;

    // Input of for the Vertex shader. This is a vertex in a geometry with all its attributes.
    struct Vertex
    {
        // 3D position in model space.
        glm::vec4 position;
        // Normal vector in model space.
        glm::vec3 normal;
        // RGBA color.
        glm::vec4 color;
        // u/v texture coordinates.
        glm::vec2 texcoord;

        inline Vertex() : position(0, 0, 0, 1), normal(0, 0, 0), color(1, 1, 1, 1), texcoord(0, 0) {}

        inline explicit Vertex(const glm::vec4 &pos) : position(pos), normal(0, 0, 0), color(0, 0, 0, 1),
                                                       texcoord(0, 0) {}

        inline Vertex(const glm::vec4 &pos, const glm::vec3 &n, const glm::vec4 &col, const glm::vec2 &tc) :
                position(pos), normal(n), color(col), texcoord(tc) {}
    };

    // The output of a vertex shader -- the transformed vertex.
    struct VertexOut
    {
        // After view and projection transform
        glm::vec4 clipPosition;

        // For shading -- position and normal in world coords
        glm::vec3 worldPosition;
        glm::vec3 worldNormal;

        // Material info here. Right now we only store the color and texture coordinates.
        glm::vec4 color;
        glm::vec2 texcoord;
    };

    // Linearly interpolates between two vertexouts.
    VertexOut &&lerp(const VertexOut &a, const VertexOut &b, float d);

    struct PointPrimitive
    {
        VertexOut p;

        inline explicit PointPrimitive(const VertexOut &o) : p(o) {}

        ShadingGeometry &&rasterize() const;
    };

    struct LinePrimitive
    {
        VertexOut a, b;

        inline LinePrimitive(const VertexOut &a_, const VertexOut &b_) : a(a_), b(b_) {};

        ShadingGeometry &&rasterize(float d) const;
    };

    struct TrianglePrimitive
    {
        VertexOut a, b, c;

        inline TrianglePrimitive(const VertexOut &a_, const VertexOut &b_, const VertexOut &c_) : a(a_), b(b_), c(c_) {}

        ShadingGeometry &&rasterize(const glm::vec3 &bary) const;

        inline void setColor(const glm::vec4 &color)
        {
            a.color = color;
            b.color = color;
            c.color = color;
        }
    };

    // Shading Geometry is the input of a fragment shader.
    struct ShadingGeometry
    {
        // world position and normal
        glm::vec3 position;
        glm::vec3 normal;

        glm::vec4 color;
        glm::vec2 texcoord;

        glm::ivec2 windowCoord;
        float depth;
    };

    // Linearly interpolates between two ShadingGeometries.
    ShadingGeometry &&interpolate(const ShadingGeometry &a, const ShadingGeometry &b, float d);

    // Final fragment shader output that will be written into a framebuffer.
    struct Fragment
    {
        glm::vec4 color;
    };


    // Lists of stuffs.
    typedef std::vector<Vertex> VertexList;
    typedef std::vector<unsigned int> IndexList;
    typedef std::vector<VertexOut> VertexOutList;

    typedef std::vector<PointPrimitive> PointPrimitiveList;
    typedef std::vector<LinePrimitive> LinePrimitiveList;
    typedef std::vector<TrianglePrimitive> TrianglePrimitiveList;
};

#endif //SRENDER_PIPELINE_H
