#include "Rasterizer.h"
#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Viewport.h"

#include "Shader.h"
#include "Pipeline.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

using glm::ivec2;
using glm::vec3;
using glm::vec4;

using namespace render;

unsigned int
Rasterizer::drawPoints(const RenderConfig &renderConfig, const VertexList &vertices, const IndexList &indices) const
{
    if (!renderConfig.isValid()) {
        std::cerr << "Invalid render configuration!\n";
        return 0;
    }

    unsigned int pointsDrawn = 0;

    // Vertex transform.
    VertexOutList transformedVertices = transformVertices(vertices, renderConfig.vertexShader);

    // Primitive assembly
    PointPrimitiveList points;
    for (size_t i = 0; i < indices.size(); ++i) {
        points.push_back(PointPrimitive(transformedVertices[indices[i]]));
    }

    // Clipping
    PointPrimitiveList clipped = clipper.clipPointsToNdc(points);

    // Perspective divide
    for (auto &p : clipped) {
        if (p.p.clipPosition.w <= 0) {
            std::cout << "p: " << p.p.clipPosition << std::endl;
        }
        p.p.clipPosition /= p.p.clipPosition.w;
    }

    // Rasterization
    for (const auto &p : clipped) {
        const vec3 pos_win = renderConfig.viewport->calculateWindowCoordinates(p.p.clipPosition);

        // If there is a depth buffer but the depth test fails -> discard fragment (early)
        if (renderConfig.depthbuffer && !renderConfig.depthbuffer->conditionalPlot(pos_win))
            continue;

        // calculate shading geometry
        ShadingGeometry sgeo = p.rasterize();
        sgeo.windowCoord = ivec2(pos_win);
        sgeo.depth = pos_win.z;

        // shade fragment and plot
        Fragment frag = renderConfig.fragmentShader->shadeSingle(sgeo);
        renderConfig.framebuffer->plot(sgeo.windowCoord, frag.color);

        ++pointsDrawn;
    }

    return pointsDrawn;
}

VertexOutList
Rasterizer::transformVertices(const VertexList &vertices, std::shared_ptr<VertexShader> vertexShader) const
{
    assert(vertexShader);
    VertexOutList out(vertices.size());
    std::transform(
            vertices.begin(),
            vertices.end(),
            out.begin(),
            [vertexShader](const auto &v) { return vertexShader->transformSingle(v); });
    return out;
}


static inline bool insideClipSpace(const VertexOut &v)
{
    return v.clipPosition.x >= -1 && v.clipPosition.x <= 1 &&
           v.clipPosition.y >= -1 && v.clipPosition.y <= 1 &&
           v.clipPosition.z >= -1 && v.clipPosition.z <= 1;
}

unsigned int Rasterizer::drawLines(const RenderConfig &renderConfig, const VertexList &vertices, const IndexList &indices) const
{
    if (!renderConfig.isValid()) {
        std::cerr << "Invalid render configuration!\n";
        return 0;
    }

    unsigned int linesDrawn = 0;

    // Vertex transformation
    VertexOutList transformedVertices = transformVertices(vertices, renderConfig.vertexShader);

    for (const auto &v : transformedVertices) {
        if (!insideClipSpace(v)) {
            //std::clog << "Outside: " << v.clipPosition << std::endl;
        }
    }

    // Primitive assembly
    LinePrimitiveList lines;
    for (size_t i = 0; i < indices.size(); i += 2) {
        const VertexOut &a = transformedVertices[indices[i + 0]];
        const VertexOut &b = transformedVertices[indices[i + 1]];
        lines.push_back(LinePrimitive(a, b));
    }

    // Clipping
    LinePrimitiveList clipped = clipper.clipLines(lines);

    // Perspective divide
    for (auto &line : clipped) {

        if (line.a.clipPosition.w < 0 || line.b.clipPosition.w < 0) {
            std::cout << "a: " << line.a.clipPosition << " b: " << line.b.clipPosition << std::endl;
        }

        line.a.clipPosition /= line.a.clipPosition.w;
        line.b.clipPosition /= line.b.clipPosition.w;
    }

    // Rasterization
    for (const auto &line : clipped) {
        drawLine(renderConfig, line);
        ++linesDrawn;
    }

    return linesDrawn;
}

unsigned int Rasterizer::drawLineStrip(const RenderConfig &renderConfig, const render::VertexList &vertices, const render::IndexList &indices) const
{
    // We can reuse the existing code by expanding the current indices. We expand the indices by doubling the internal
    // vertices; i.e. [0,2,4,6] -> [0,2,2,4,4,6]
    render::IndexList expandedIndices(indices.size() - 2);

    for (size_t i = 0; i < indices.size() - 1; ++i) {
        expandedIndices.push_back(indices[i + 0]);
        expandedIndices.push_back(indices[i + 1]);
    }

    return drawLines(renderConfig, vertices, expandedIndices);
}

unsigned int Rasterizer::drawTriangles(const RenderConfig &renderConfig, const VertexList &vertices, const IndexList &indices) const
{
    if (!renderConfig.isValid()) {
        std::cerr << "Invalid render configuration!\n";
        return 0;
    }

    unsigned int trianglesDrawn = 0;

    // transform vertices
    VertexOutList transformedVertices = transformVertices(vertices, renderConfig.vertexShader);

    // Primitive assembly.
    TrianglePrimitiveList triangles;
    // https://www.gamasutra.com/view/news/168577/Indepth_Software_rasterizer_and_triangle_clipping.php
    // https://fgiesen.wordpress.com/2011/07/05/a-trip-through-the-graphics-pipeline-2011-part-5/
    for (size_t i = 0; i < indices.size(); i += 3) {
        const VertexOut &a = transformedVertices[indices[i + 0]];
        const VertexOut &b = transformedVertices[indices[i + 1]];
        const VertexOut &c = transformedVertices[indices[i + 2]];
        triangles.push_back(TrianglePrimitive(a, b, c));
    }

    // Perspective divide
    for (auto &triangle : triangles) {
        triangle.a.clipPosition /= triangle.a.clipPosition.w;
        triangle.b.clipPosition /= triangle.b.clipPosition.w;
        triangle.c.clipPosition /= triangle.c.clipPosition.w;

        // Add backface culling here
    }

    // At this point all triangles are in clip space [-1 .. 1] and can be clipped to NDC.
    TrianglePrimitiveList clipped = clipper.clipTrianglesToNdc(triangles);

    for (const TrianglePrimitive &triangle : clipped) {
        drawTriangle(renderConfig, triangle);
        ++trianglesDrawn;
    }

    return trianglesDrawn;
}

// Bresenham line drawing
void Rasterizer::drawLine(const RenderConfig &renderConfig, const LinePrimitive &line) const
{
    assert(renderConfig.fragmentShader);

    using namespace glm;

    const vec3 posA_win = renderConfig.viewport->calculateWindowCoordinates(line.a.clipPosition);
    const vec3 posB_win = renderConfig.viewport->calculateWindowCoordinates(line.b.clipPosition);

    ivec2 a = ivec2(posA_win);
    ivec2 b = ivec2(posB_win);

    float lineLength = sqrtf((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
    unsigned int positionCounter = 0;

    int dx = abs(a.x - b.x), sx = a.x < b.x ? 1 : -1;
    int dy = abs(a.y - b.y), sy = a.y < b.y ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;) {
        // interpolation for depth and texturing/color
        float delta = std::min(1.f, positionCounter / lineLength);
        float depth = mix(posA_win.z, posB_win.z, delta);

        if (renderConfig.viewport->isInside(a) &&
            (!renderConfig.depthbuffer || (renderConfig.depthbuffer && renderConfig.depthbuffer->conditionalPlot(a.x, a.y, depth)))) {
            ShadingGeometry sgeo = line.rasterize(positionCounter / lineLength);
            sgeo.windowCoord = a;
            sgeo.depth = depth;

            if (renderConfig.framebuffer) {
                Fragment frag = renderConfig.fragmentShader->shadeSingle(sgeo);
                renderConfig.framebuffer->plot(a, frag.color);
            }
        }

        // 'Core' Bresenham algorithm.
        if (a.x == b.x && a.y == b.y)
            break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            a.x += sx;
        }
        if (e2 < dy) {
            err += dx;
            a.y += sy;
        }

        ++positionCounter;
    }
}

// Finds which part of the half-space of line a-b point c is in (positive or negative)
static inline int pointInHalfspace(const glm::ivec2 &a, const glm::ivec2 &b, const glm::ivec2 &c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// Parameter-based rasterization of triangles. It calculates the screen-space bounding box of the triangle, then
// checks every contained pixel whether it's in the triangle or not. If the fragment is inside, it proceeds to the
// depth test and shading stage.
void Rasterizer::drawTriangle(const RenderConfig &renderConfig, const TrianglePrimitive &t) const
{
    assert(renderConfig.fragmentShader);

    using namespace glm;

    const vec3 posA_win = renderConfig.viewport->calculateWindowCoordinates(t.a.clipPosition);
    const vec3 posB_win = renderConfig.viewport->calculateWindowCoordinates(t.b.clipPosition);
    const vec3 posC_win = renderConfig.viewport->calculateWindowCoordinates(t.c.clipPosition);

    // three window/screen coordinates
    ivec2 a = ivec2(posA_win);
    ivec2 b = ivec2(posB_win);
    ivec2 c = ivec2(posC_win);

    // calculate bounds
    ivec2 min, max;
    min.x = glm::min(a.x, glm::min(b.x, c.x));
    min.y = glm::min(a.y, glm::min(b.y, c.y));
    max.x = glm::max(a.x, glm::max(b.x, c.x));
    max.y = glm::max(a.y, glm::max(b.y, c.y));

    // clip against screen coords
    min = glm::max(renderConfig.viewport->origin, min);
    max = glm::min(renderConfig.viewport->origin + renderConfig.viewport->size - 1, max);

    if (renderConfig.drawTriangleBounds && renderConfig.framebuffer) {
        // draw bounding box
        vec4 bboxColour(1, 0, 0, 1);
        for (int x = min.x; x <= max.x; ++x) {
            ivec2 p(x, min.y);
            renderConfig.framebuffer->plot(p, bboxColour);
            p = ivec2(x, max.y);
            renderConfig.framebuffer->plot(p, bboxColour);
        }

        for (int y = min.y; y <= max.y; ++y) {
            ivec2 p(min.x, y);
            renderConfig.framebuffer->plot(p, bboxColour);
            p = ivec2(max.x, y);
            renderConfig.framebuffer->plot(p, bboxColour);
        }
    }

    // Rasterize -- loop over the screen-space bounding box.
    for (int y = min.y; y <= max.y; ++y) {
        for (int x = min.x; x <= max.x; ++x) {
            // position
            ivec2 p(x, y);

            int w0 = pointInHalfspace(b, c, p);
            int w1 = pointInHalfspace(c, a, p);
            int w2 = pointInHalfspace(a, b, p);

            // determine barycentric coords
            vec3 lambda;
            float w = (float) (w0 + w1 + w2);

            lambda.x = (float) w0 / w;
            lambda.y = (float) w1 / w;
            lambda.z = (float) w2 / w;

            // calculate depth here
            float z = lambda.x * posA_win.z + lambda.y * posB_win.z + lambda.z * posC_win.z;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {

                // depth test
                // TODO: This needs to be improved. Depth buffer must be made optional.
                if (renderConfig.depthbuffer && (renderConfig.depthbuffer->conditionalPlot(x, y, z))) {
                    ShadingGeometry sgeo = t.rasterize(lambda);
                    sgeo.windowCoord = p;
                    sgeo.depth = z;

                    if (renderConfig.framebuffer) {
                        Fragment frag = renderConfig.fragmentShader->shadeSingle(sgeo);
                        renderConfig.framebuffer->plot(p, frag.color);
                    }
                }

            }

        }
    }

}
