#include "Rasterizer.h"
#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Viewport.h"

#include "Shader.h"
#include "Pipeline.h"

#include <algorithm>
#include <iostream>
#include <list>

using glm::ivec2;
using glm::vec3;
using glm::vec4;

namespace render
{

// a helper struct/functor object for STL algorithms
    struct CallVertexShader
    {
        std::shared_ptr<VertexShader> shader;

        inline CallVertexShader(std::shared_ptr<VertexShader> sh) : shader(sh) {}

        inline VertexOut operator()(const Vertex &in)
        {
            return shader->transformSingle(in);
        };
    };

    bool Rasterizer::ShaderConfiguration::isValid() const
    {
        return vertexShader && fragmentShader;
    }

    Rasterizer::Rasterizer() : drawBoundingBoxes(false)
    {
    }

    unsigned int Rasterizer::drawPoints(const VertexList &vertices) const
    {
        if (!shaders.isValid()) {
            std::cerr << "Invalid shader configuration!\n";
            return 0;
        }

        unsigned int pointsDrawn = 0;

        VertexOutList transformedVertices(vertices.size());
        transformVertices(vertices, transformedVertices, shaders.vertexShader);


        // build points
        PointPrimitiveList points;
        for (VertexOutList::const_iterator po = transformedVertices.begin(); po != transformedVertices.end(); ++po) {
            PointPrimitive pt(*po);

            // check clipping/culling here
            if (pt.clipToNDC() == DISCARD)
                continue;

            points.push_back(pt);
        }

        for (PointPrimitiveList::const_iterator it = points.begin(); it != points.end(); ++it) {
            const PointPrimitive &p = *it;

            // perspective divide and window coordinates
            const vec4 &pos_clip = p.p.clipPosition;
            const vec3 pos_ndc = vec3(pos_clip) / pos_clip.w;
            const vec3 pos_win = output.viewport->calculateWindowCoordinates(pos_ndc);

            // if there is a depth buffer but the depth test fails -> discard fragment (early)
            if (output.depthbuffer && !output.depthbuffer->conditionalPlot(pos_win))
                continue;


            // calculate shading geometry
            ShadingGeometry sgeo = p.rasterise();
            sgeo.windowCoord = ivec2(pos_win);
            sgeo.depth = pos_win.z;

            // shade fragment and plot
            Fragment frag = shaders.fragmentShader->shadeSingle(sgeo);
            output.framebuffer->plot(sgeo.windowCoord, frag.color);

            ++pointsDrawn;
        }

        return pointsDrawn;
    }

    void Rasterizer::transformVertices(const VertexList &vertices, VertexOutList &out,
                                       std::shared_ptr<VertexShader> vertexShader) const
    {
        assert(vertexShader);
        CallVertexShader vertexTransform(vertexShader);
        std::transform(vertices.begin(), vertices.end(), out.begin(), vertexTransform);
    }

    unsigned int Rasterizer::drawLines(const VertexList &vertices, const IndexList &indices) const
    {
        if (!shaders.isValid()) {
            std::cerr << "Invalid shader configuration!\n";
            return 0;
        }

        unsigned int linesDrawn = 0;

        // transform vertices
        VertexOutList transformedVertices(vertices.size());
        transformVertices(vertices, transformedVertices, shaders.vertexShader);

        // build lines
        LinePrimitiveList lines;
        for (size_t i = 0; i < indices.size(); i += 2) {
            const VertexOut &a = transformedVertices[indices[i + 0]];
            const VertexOut &b = transformedVertices[indices[i + 1]];
            lines.push_back(LinePrimitive(a, b));
        }

        for (auto line : lines) {
            // clip and cull lines here
            ClipResult cr = line.clipToNDC();
            if (cr == DISCARD) {
                // line completely outside -- discard it
                continue;
            }

            drawLine(line);
            ++linesDrawn;
        }

        return linesDrawn;
    }

    unsigned int Rasterizer::drawTriangles(const VertexList &vertices, const IndexList &indices) const
    {
        if (!shaders.isValid()) {
            std::cerr << "Invalid shader configuration!\n";
            return 0;
        }

        unsigned int trianglesDrawn = 0;

        // transform vertices
        VertexOutList transformedVertices(vertices.size());
        transformVertices(vertices, transformedVertices, shaders.vertexShader);

        // build triangles
        TrianglePrimitiveList triangles;
        for (size_t i = 0; i < indices.size(); i += 3) {
            const VertexOut &a = transformedVertices[indices[i + 0]];
            const VertexOut &b = transformedVertices[indices[i + 1]];
            const VertexOut &c = transformedVertices[indices[i + 2]];

            // backface culling in NDC
            vec3 a_ndc = vec3(a.clipPosition / a.clipPosition.w);
            vec3 b_ndc = vec3(b.clipPosition / b.clipPosition.w);
            vec3 c_ndc = vec3(c.clipPosition / c.clipPosition.w);

            vec3 n = cross(b_ndc - a_ndc, c_ndc - a_ndc);
            if (n.z >= 0)
                triangles.push_back(TrianglePrimitive(a, b, c));
        }

        for (auto tri = triangles.begin(); tri != triangles.end(); ++tri) {
            // clip and cull triangles here
            ClipResult cr = tri->clipToNDC();
            if (cr == DISCARD) {
                // triangle completely outside -- discard it
                continue;
            }

            if (cr == CLIPPED) {
                // TODO: insert newly created triangles at the end

            }

            // rasterize and interpolate triangle here
            drawTriangle(*tri);
        }

        return trianglesDrawn;
    }

// Bresenham line drawing
    void Rasterizer::drawLine(const LinePrimitive &line) const
    {
        assert(shaders.fragmentShader);

        using namespace glm;

        // point a
        const vec4 &posA_clip = line.a.clipPosition;
        const vec3 posA_ndc = vec3(posA_clip) / posA_clip.w;
        const vec3 posA_win = output.viewport->calculateWindowCoordinates(posA_ndc);

        // point b
        const vec4 &posB_clip = line.b.clipPosition;
        const vec3 posB_ndc = vec3(posB_clip) / posB_clip.w;
        const vec3 posB_win = output.viewport->calculateWindowCoordinates(posB_ndc);

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

            if (output.viewport->isInside(a) &&
                (!output.depthbuffer || (output.depthbuffer && output.depthbuffer->conditionalPlot(a.x, a.y, depth)))) {
                ShadingGeometry sgeo = line.rasterise(positionCounter / lineLength);
                sgeo.windowCoord = a;
                sgeo.depth = depth;

                if (output.framebuffer) {
                    Fragment frag = shaders.fragmentShader->shadeSingle(sgeo);
                    output.framebuffer->plot(a, frag.color);
                }
            }

            // 'core' bresenham
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
    void Rasterizer::drawTriangle(const TrianglePrimitive &t) const
    {
        assert(shaders.fragmentShader);

        using namespace glm;
        // point a
        const vec4 &posA_clip = t.a.clipPosition;
        const vec3 posA_ndc = vec3(posA_clip) / posA_clip.w;
        const vec3 posA_win = output.viewport->calculateWindowCoordinates(posA_ndc);

        // point b
        const vec4 &posB_clip = t.b.clipPosition;
        const vec3 posB_ndc = vec3(posB_clip) / posB_clip.w;
        const vec3 posB_win = output.viewport->calculateWindowCoordinates(posB_ndc);

        // point c
        const vec4 &posC_clip = t.c.clipPosition;
        const vec3 posC_ndc = vec3(posC_clip) / posC_clip.w;
        const vec3 posC_win = output.viewport->calculateWindowCoordinates(posC_ndc);

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
        min = glm::max(output.viewport->origin, min);
        max = glm::min(output.viewport->origin + output.viewport->size - 1, max);

        if (drawBoundingBoxes && output.framebuffer) {
            // draw bounding box
            vec4 bboxColour(1, 0, 0, 1);
            for (int x = min.x; x <= max.x; ++x) {
                ivec2 p(x, min.y);
                output.framebuffer->plot(p, bboxColour);
                p = ivec2(x, max.y);
                output.framebuffer->plot(p, bboxColour);
            }

            for (int y = min.y; y <= max.y; ++y) {
                ivec2 p(min.x, y);
                output.framebuffer->plot(p, bboxColour);
                p = ivec2(max.x, y);
                output.framebuffer->plot(p, bboxColour);
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
                float w = (float)(w0 + w1 + w2);

                lambda.x = (float) w0 / w;
                lambda.y = (float) w1 / w;
                lambda.z = (float) w2 / w;

                // calculate depth here
                float z = lambda.x * posA_win.z + lambda.y * posB_win.z + lambda.z * posC_win.z;

                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {

                    // depth test
                    // TODO: This needs to be improved. Depth buffer must be made optional.
                    if (output.depthbuffer && (output.depthbuffer->conditionalPlot(x, y, z))){
                        ShadingGeometry sgeo = t.rasterise(lambda);
                        sgeo.windowCoord = p;
                        sgeo.depth = z;

                        if (output.framebuffer) {
                            Fragment frag = shaders.fragmentShader->shadeSingle(sgeo);
                            output.framebuffer->plot(p, frag.color);
                        }
                    }

                }

            }
        }

    }

    void Rasterizer::toggleBoundingBoxes()
    {
        drawBoundingBoxes = !drawBoundingBoxes;
        std::clog << (drawBoundingBoxes ? "D" : "Not d") << "rawing bounding boxes." << std::endl;
    }

}