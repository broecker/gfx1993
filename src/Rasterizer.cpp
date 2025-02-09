#include "Rasterizer.h"

#include "config.h"
#include "Depthbuffer.h"
#include "Framebuffer.h"
#include "Viewport.h"
#include "Shader.h"

#include <algorithm>
#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <list>
#include <omp.h>

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;

namespace gfx1993 {

#if GFX1993_ENABLE_PROFILING
  #if GFX1993_ENABLE_DEBUG_PROFILING
    #define START_PROFILE(name) const auto profile_##name_start = profileInfo.startTiming(name); std::clog << "[Rasterizer stage]: " << name << std::endl;
  #else
    #define START_PROFILE(name) const auto profile_##name_start = profileInfo.startTiming(name)    
  #endif // GFX1993_ENABLE_DEBUG_PROFILING
#else 
  #define START_PROFILE(name) {}
#endif

#define SAVE_COUNTER(rasterFunction, debugCounter) if (rasterFunction) {debugCounter.fragmentsDrawn++;} else {debugCounter.fragmentsDiscarded++;} 

void Rasterizer::drawPoints(const RenderConfig &renderConfig,
                            const VertexList &vertices,
                            const IndexList &indices) const {
  if (!renderConfig.isValid()) {
    std::cerr << "Invalid render configuration!\n";
  }
  START_PROFILE("rasterize.points");
  debugInfo.points.processed += indices.size();

  // Vertex transform.
  VertexOutList transformedVertices;
  {
    START_PROFILE("rasterize.points.transform");
    transformedVertices = transformVertices(vertices, renderConfig.vertexShader);
  }

  // Primitive assembly
  PointPrimitiveList points;
  {
    START_PROFILE("rasterize.points.assembly");
    for (size_t i = 0; i < indices.size(); ++i) {
      points.push_back(PointPrimitive(transformedVertices[indices[i]]));
    }
  }

  // Clipping
  PointPrimitiveList clipped;
  {
    START_PROFILE("rasterize.points.clip");
    clipped = clipper.clipPointsToNdc(points);


    // Perspective divide
    for (auto &p : clipped) {
      if (p.p.clipPosition.w <= 0) {
        std::cout << "p: " << p.p.clipPosition << std::endl;
      }
      p.p.clipPosition /= p.p.clipPosition.w;
    }
  }

  // Rasterization
  {
    START_PROFILE("rasterize.points.shade");
    for (const auto &p : clipped) {
      const vec3 pos_win =
          renderConfig.viewport->calculateWindowCoordinates(p.p.clipPosition);

      // If there is a depth buffer but the depth test fails -> discard fragment
      // (early)
      if (renderConfig.depthbuffer &&
          !renderConfig.depthbuffer->isVisible(pos_win, pos_win.z))
        continue;

      // Enable 'fat' points here.
      if (renderConfig.pointSize > 1) {
        int halfSize = (renderConfig.pointSize-1) / 2;
        for (int x = -halfSize; x <= halfSize; ++x) {
          for (int y = -halfSize; y <= halfSize; ++y) {
            // calculate shading geometry
            ShadingGeometry sgeo = p.rasterize();
            sgeo.windowCoord = ivec2(pos_win) + ivec2(x,y);
            sgeo.depth = pos_win.z;

            if (!renderConfig.viewport->isInside(sgeo.windowCoord)) {
              continue;
            }

            // shade fragment and plot
            SAVE_COUNTER(drawFragment(renderConfig, sgeo), debugInfo.points);
          }
        }
      } else {
        ShadingGeometry sgeo = p.rasterize();
          sgeo.windowCoord = ivec2(pos_win);
          sgeo.depth = pos_win.z;
        // shade fragment and plot
        SAVE_COUNTER(drawFragment(renderConfig, sgeo), debugInfo.points);
      }
      debugInfo.points.drawn++;
    }    
  }
}

VertexOutList Rasterizer::transformVertices(
    const VertexList &vertices,
    std::shared_ptr<VertexShader> vertexShader) const {
  assert(vertexShader);
  VertexOutList out(vertices.size());

#if GFX1993_PARALLEL_TRANSFORM
  #pragma omp parallel for
  for (int i = 0; i < vertices.size(); ++i) {
    out[i] = vertexShader->transformSingle(vertices[i]);
  }
#else
  std::transform(vertices.begin(), vertices.end(), out.begin(),
                 [vertexShader](const auto &v) {
                   return vertexShader->transformSingle(v);
                 });
#endif
  return out;
}

static inline bool insideClipSpace(const VertexOut &v) {
  return v.clipPosition.x >= -1 && v.clipPosition.x <= 1 &&
         v.clipPosition.y >= -1 && v.clipPosition.y <= 1 &&
         v.clipPosition.z >= -1 && v.clipPosition.z <= 1;
}

void Rasterizer::drawLines(const RenderConfig &renderConfig,
                           const VertexList &vertices,
                           const IndexList &indices) const {
  if (!renderConfig.isValid()) {
    std::cerr << "Invalid render configuration!\n";
    return;
  }

  if (indices.size() % 2 == 1) {
    std::cerr << "Invalid indices; expected an even number, got: " << indices.size() << std::endl;
    return;
  }

  START_PROFILE("rasterize.lines");
  debugInfo.lines.processed++;

  // Vertex transformation
  VertexOutList transformedVertices;
  transformedVertices.reserve(vertices.size());
  {
    START_PROFILE("rasterize.lines.transform");
    transformedVertices = transformVertices(vertices, renderConfig.vertexShader);
  }

  // Primitive assembly
  LinePrimitiveList lines;
  lines.reserve(indices.size()/2);
  {
    START_PROFILE("rasterize.lines.assembly");
    for (size_t i = 0; i < indices.size(); i += 2) {
      const VertexOut &a = transformedVertices[indices[i + 0]];
      const VertexOut &b = transformedVertices[indices[i + 1]];
      lines.push_back(LinePrimitive(a, b));
    }
  }

  // Clipping
  LinePrimitiveList clipped;
  {
    START_PROFILE("rasterize.lines.clip");
    clipped = clipper.clipLines(lines);

    // Perspective divide
    for (auto &line : clipped) {
      if (line.a.clipPosition.w < 0 || line.b.clipPosition.w < 0) {
        std::cout << "a: " << line.a.clipPosition << " b: " << line.b.clipPosition
                  << std::endl;
      }

      line.a.clipPosition /= line.a.clipPosition.w;
      line.b.clipPosition /= line.b.clipPosition.w;
    }
  }

  // Rasterization
  for (const auto &line : clipped) {
    drawLine(renderConfig, line);
  }
}

void Rasterizer::drawLineStrip(const RenderConfig &renderConfig,
                               const VertexList &vertices,
                               const IndexList &indices) const {
  // We can reuse the existing code by expanding the current indices. We expand
  // the indices by doubling the internal vertices; i.e. [0,2,4,6] ->
  // [0,2,2,4,4,6]
  IndexList expandedIndices;

  for (size_t i = 0; i < indices.size() - 1; ++i) {
    expandedIndices.push_back(indices[i + 0]);
    expandedIndices.push_back(indices[i + 1]);
  }

  drawLines(renderConfig, vertices, expandedIndices);
}

void Rasterizer::drawTriangles(const RenderConfig &renderConfig,
                               const VertexList &vertices,
                               const IndexList &indices) const {
  if (!renderConfig.isValid()) {
    std::cerr << "Invalid render configuration!\n";
    return;
  }
  START_PROFILE("rasterize.tris");
  debugInfo.triangles.processed++;

  // transform vertices
  VertexOutList transformedVertices;
  {
    START_PROFILE("rasterize.tris.transform");
    transformedVertices =
        transformVertices(vertices, renderConfig.vertexShader);
  }

  // Primitive assembly.
  TrianglePrimitiveList triangles;
  {
    START_PROFILE("rasterize.tris.assembly");
    
    // https://www.gamasutra.com/view/news/168577/Indepth_Software_rasterizer_and_triangle_clipping.php
    // https://fgiesen.wordpress.com/2011/07/05/a-trip-through-the-graphics-pipeline-2011-part-5/
    for (size_t i = 0; i < indices.size(); i += 3) {
      const VertexOut &a = transformedVertices[indices[i + 0]];
      const VertexOut &b = transformedVertices[indices[i + 1]];
      const VertexOut &c = transformedVertices[indices[i + 2]];
      triangles.push_back(TrianglePrimitive(a, b, c));
    }
  }

  // At this point all triangles are in clip space [-1 .. 1] and can be clipped
  // to NDC.
  TrianglePrimitiveList clipped;
  IndexList trianglesToDraw;
  {
    START_PROFILE("rasterize.tris.clip");
    clipped = clipper.clipTrianglesToNdc(triangles);

    // Perspective divide
    for (size_t i = 0; i < clipped.size(); ++i) {
      auto& triangle = clipped[i];
      triangle.a.clipPosition /= triangle.a.clipPosition.w;
      triangle.b.clipPosition /= triangle.b.clipPosition.w;
      triangle.c.clipPosition /= triangle.c.clipPosition.w;

      if (renderConfig.cullBackFaces) {
        START_PROFILE("rasterize.tris.clip.cullface");
        vec3 clipNormal = glm::normalize(glm::cross(vec3(triangle.b.clipPosition) - vec3(triangle.a.clipPosition),
                                                      vec3(triangle.c.clipPosition) - vec3(triangle.a.clipPosition)));
        if (clipNormal.z <= 0) {
          debugInfo.triangles.backfaceCulled++;
        } else {
          trianglesToDraw.push_back(i);  
        }
      } else {
        trianglesToDraw.push_back(i);
      }

    }
  }

  // Rasterization.
  for (size_t i = 0; i < trianglesToDraw.size(); ++i) {
    drawTriangle(renderConfig, clipped[trianglesToDraw[i]]);
  }
}


void Rasterizer::drawScreenFillingQuad(const RenderConfig& renderConfig) {
  if (!renderConfig.isValid()) {
    std::cerr << "Invalid render configuration!\n";
    return;
  }

  START_PROFILE("rasterize.screenQuad");
  debugInfo.screenFillingQuad.processed++;
  debugInfo.screenFillingQuad.drawn++;

#if GFX1993_PARALLEL_SHADE_SCREENQUAD
  #pragma omp parallel for
  for (int y = 0; y < renderConfig.viewport->size.y; ++y) {
#else
  for (int y = 0; y < renderConfig.viewport->size.y; ++y) {
#endif
    for (int x = 0; x < renderConfig.viewport->size.x; ++x) {
      ShadingGeometry sgeo;
      sgeo.color = vec4(1);
      sgeo.normal = vec3(0);
      sgeo.windowCoord = glm::ivec2(x,y) + renderConfig.viewport->origin;

      vec2 pos = vec2(x,y) / vec2(renderConfig.viewport->size.x, renderConfig.viewport->size.y);
      sgeo.position = vec3(pos, 0.0);
      sgeo.texcoord = pos;

      bool drawn = drawFragment(renderConfig, sgeo);
      {
        #pragma omp critcal
        SAVE_COUNTER(drawn, debugInfo.screenFillingQuad);
      }
    }
  }
}

// Bresenham line drawing
void Rasterizer::drawLine(const RenderConfig &renderConfig,
                          const LinePrimitive &line) const {
  assert(renderConfig.fragmentShader);
  START_PROFILE("rasterize.lines.draw");

  using namespace glm;

  const vec3 posA_win =
      renderConfig.viewport->calculateWindowCoordinates(line.a.clipPosition);
  const vec3 posB_win =
      renderConfig.viewport->calculateWindowCoordinates(line.b.clipPosition);

  ivec2 a = ivec2(posA_win);
  ivec2 b = ivec2(posB_win);

  float lineLength =
      sqrtf((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
  unsigned int positionCounter = 0;

  if (lineLength == 0) {
    // Invalid line.
    // TODO(mbroecker): Report error?
    return;
  }

  int dx = abs(a.x - b.x), sx = a.x < b.x ? 1 : -1;
  int dy = abs(a.y - b.y), sy = a.y < b.y ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2, e2;

  for (;;) {
    // interpolation for depth and texturing/color
    float delta = std::min(1.f, positionCounter / lineLength);
    float depth = mix(posA_win.z, posB_win.z, delta);

    ShadingGeometry sgeo = line.rasterize(positionCounter / lineLength);
    sgeo.windowCoord = a;
    sgeo.depth = depth;

    {
      START_PROFILE("rasterize.lines.shade");
      SAVE_COUNTER(drawFragment(renderConfig, sgeo), debugInfo.lines);
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
  debugInfo.lines.drawn++;
}

// Finds which part of the half-space of line a-b point c is in (positive or
// negative)
static inline int pointInHalfspace(const glm::ivec2 &a, const glm::ivec2 &b,
                                   const glm::ivec2 &c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// Parameter-based rasterization of triangles. It calculates the screen-space
// bounding box of the triangle, then checks every contained pixel whether it's
// in the triangle or not. If the fragment is inside, it proceeds to the depth
// test and shading stage.
void Rasterizer::drawTriangle(const RenderConfig &renderConfig,
                              const TrianglePrimitive &t) const {
  assert(renderConfig.fragmentShader);
  START_PROFILE("rasterize.tris.shade");

  using namespace glm;

  const vec3 posA_win =
      renderConfig.viewport->calculateWindowCoordinates(t.a.clipPosition);
  const vec3 posB_win =
      renderConfig.viewport->calculateWindowCoordinates(t.b.clipPosition);
  const vec3 posC_win =
      renderConfig.viewport->calculateWindowCoordinates(t.c.clipPosition);

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
  max = glm::min(
      renderConfig.viewport->origin + renderConfig.viewport->size - 1, max);

  if (renderConfig.drawTriangleBounds && renderConfig.framebuffer) {
    START_PROFILE("rasterize.tris.shade.bbox");
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
  {
    START_PROFILE("rasterize.tris.shade.fill");
#if GFX1993_PARALLEL_SHADE_TRIANGLE
    // TODO(mbroecker): Maybe an additional metric would be the size of the
    // bounding box and whether one dimension is much larger than the other.
    #pragma omp parallel for
    for (int y = min.y; y <= max.y; ++y) {
#else
    for (int y = min.y; y <= max.y; ++y) {
#endif
      for (int x = min.x; x <= max.x; ++x) {
        // position
        ivec2 p(x, y);

        int w0 = pointInHalfspace(b, c, p);
        int w1 = pointInHalfspace(c, a, p);
        int w2 = pointInHalfspace(a, b, p);

        // determine barycentric coords
        vec3 lambda;
        float w = (float)(w0 + w1 + w2);

        lambda.x = (float)w0 / w;
        lambda.y = (float)w1 / w;
        lambda.z = (float)w2 / w;

        // calculate depth here
        float z =
            lambda.x * posA_win.z + lambda.y * posB_win.z + lambda.z * posC_win.z;

        if (w0 <= 0 && w1 <= 0 && w2 <= 0) {
          ShadingGeometry sgeo = t.rasterize(lambda);
          sgeo.windowCoord = p;
          sgeo.depth = z;

          bool drawn = drawFragment(renderConfig, sgeo);
          {
            #pragma criticial
            SAVE_COUNTER(drawn, debugInfo.triangles);
          }
        }
      }
    }
    #pragma omp atomic
    debugInfo.triangles.drawn++;
  }
}

bool Rasterizer::drawDepthFragment(const RenderConfig& renderConfig,
                                   const ShadingGeometry &geometry) const {
  if (!renderConfig.depthWrite) {
    return false;
  }
  if (renderConfig.depthTest) {
    return renderConfig.depthbuffer->conditionalPlot(
        geometry.windowCoord.x, geometry.windowCoord.y, geometry.depth);
  } else {
    renderConfig.depthbuffer->plot(
        geometry.windowCoord.x, geometry.windowCoord.y, geometry.depth);
    return true;
  }
}

bool Rasterizer::drawFragment(const RenderConfig &renderConfig,
                              const ShadingGeometry &geometry) const {
  // No need for shading, write to depth buffer and that's it.
  if (!renderConfig.framebuffer) {
    return drawDepthFragment(renderConfig, geometry);
  } 

  if (!renderConfig.depthbuffer ||
      !renderConfig.depthTest ||
      (renderConfig.depthbuffer &&
        renderConfig.depthbuffer->isVisible(geometry.windowCoord,
                                            geometry.depth))) {

    Fragment frag = renderConfig.fragmentShader->shadeSingle(geometry);

    // Fragment was discarded by the frag shader -- ignore and
    // keep rasterizing.
    if (frag.discard) {
      return false;
    } else {
      // Fragment is valid -- write depth now.
      if (renderConfig.depthbuffer && renderConfig.depthWrite)
        renderConfig.depthbuffer->plot(geometry.windowCoord, geometry.depth);
    }

    // If we have enabled alpha blending and have a transparent
    // fragment.
    if (renderConfig.alphaBlending && frag.color.a < 1) {
      glm::vec4 color =
          renderConfig.framebuffer->getPixel(geometry.windowCoord) *
              (1.f - frag.color.a) +
          frag.color * frag.color.a;
      renderConfig.framebuffer->plot(geometry.windowCoord, color);
    } else {
      renderConfig.framebuffer->plot(geometry.windowCoord, frag.color);
    }
    return true;
  } else {
    return false;
  }
}

}  // namespace gfx1993
