#include "Clipper.h"
#include <map>
#include <algorithm>
#include <iostream>

#include <glm/gtx/io.hpp>

namespace render {

Clipper::Clipper(const Clipper::Plane &plane) {
    planes.push_back(plane);
}

Clipper::Clipper(const std::vector<Plane> &clipPlanes) : planes(clipPlanes) {}

Clipper::Clipper() {
    using glm::vec3;
    using glm::vec4;

    // The six clip planes in NDC.
    planes.push_back(Plane(vec3(1.f,0.f,0.f),-1.f));
    planes.push_back(Plane(vec3(-1.f,0,0), -1.f));
    planes.push_back(Plane(vec3(0,1.f,0),-1.f));
    planes.push_back(Plane(vec3(0,-1.f,0),-1.f));
    planes.push_back(Plane(vec3(0,0,1.f),-1.f));
    planes.push_back(Plane(vec3(0.f,0.f,-1.f),-1.f));
}


PointPrimitiveList Clipper::clipPoints(const PointPrimitiveList &points) const {
    PointPrimitiveList clipped; clipped.reserve(points.size());

    for (const PointPrimitive& point : points) {
        bool keep = true;
        for (const Plane& p : planes) {
            if (!p.inFrontSpace(point.p.clipPosition)) {
                keep = false;
                break;
            }
        }
        if (keep) {
            clipped.push_back(point);
        }
    }
    return clipped;
}

PointPrimitiveList Clipper::clipPointsToNdc(const PointPrimitiveList& points) const {
    PointPrimitiveList clipped; clipped.reserve(points.size());

    for (const PointPrimitive& p : points) {
        if (p.p.clipPosition.x >= -p.p.clipPosition.w && p.p.clipPosition.x <= p.p.clipPosition.w &&
            p.p.clipPosition.y >= -p.p.clipPosition.w && p.p.clipPosition.y <= p.p.clipPosition.w &&
            p.p.clipPosition.z >= -p.p.clipPosition.w && p.p.clipPosition.z <= p.p.clipPosition.w)
        clipped.push_back(p);
    }
    return clipped;
}

LinePrimitiveList Clipper::clipLines(const render::LinePrimitiveList &lines) const {
    using glm::vec4;

    LinePrimitiveList clipped;

    for (LinePrimitive line : lines) {
        bool keep = true;
        for (const Clipper::Plane& plane : planes) {
            // Signed distances from the clipping planes to both line end points.
            float d0 = plane.distance(line.a.clipPosition);
            float d1 = plane.distance(line.b.clipPosition);

            // Both are outside -- discard.
            if (d0 < 0.f && d1 < 0.f) {
                keep = false;
                break;
            }
            // Both are inside -- no clipping needed.
            if (d0 >= 0.f && d1 >= 0.f) {
                continue;
            }

            // One is outside, the other inside -- calculate intersection and clip.
            float t = d0 / (d0 - d1);
            // a is inside, b outside
            if (d0 >= 0.f)
                line.b = lerp(line.a, line.b, t);
                // a is outside, b inside
            else
                line.a = lerp(line.a, line.b, t);
        }
        if (keep) {
            clipped.push_back(line);
        }
    }
    return clipped;
}

// Clips the edge ab against the given plane and returns the point on the plane.
static VertexOut clipEdge(const VertexOut& a, const VertexOut& b, const Clipper::Plane& plane) {
    float da = plane.distance(a.clipPosition);
    float db = plane.distance(b.clipPosition);

    // One is outside, the other inside -- calculate intersection and clip.
    float t = da / (da - db);
    return lerp(a, b, t);
}

static void setColor(render::TrianglePrimitive& triangle, const glm::vec4& color) {
    triangle.a.color = color;
    triangle.b.color = color;
    triangle.c.color = color;
}

TrianglePrimitiveList Clipper::clipTriangles(render::TrianglePrimitiveList triangles) const {
    TrianglePrimitiveList clipped; clipped.reserve(triangles.size());

    const static size_t MAX_TRI_COUNT = 1000;

    bool colorClips = false;

    for (size_t i = 0; i < triangles.size() && i < MAX_TRI_COUNT; ++i) {
        // Triangles might change during iteration, as new triangles are created by clipping.
        TrianglePrimitive triangle = triangles[i];



        std::vector<glm::vec4> clipPositions;
        clipPositions.push_back(triangle.a.clipPosition);
        clipPositions.push_back(triangle.b.clipPosition);
        clipPositions.push_back(triangle.c.clipPosition);

        if (std::all_of(clipPositions.begin(), clipPositions.end(), [] (const glm::vec4& v) { return v.w <= 0.f; })) {
            // All behind the w=0 axis -- discard;
            continue;
        }

        if (!std::all_of(clipPositions.begin(), clipPositions.end(), [] (const glm::vec4& v) { return v.w > 0.f; })) {
            std::clog << "Some w coordinates negative!" << std::endl;
        }

        bool keep = true;
        for (const Plane& plane : planes) {
            bool aInFrontSpace = plane.inFrontSpace(triangle.a.clipPosition);
            bool bInFrontSpace = plane.inFrontSpace(triangle.b.clipPosition);
            bool cInFrontSpace = plane.inFrontSpace(triangle.c.clipPosition);

            // All points visible, keep going.
            if (aInFrontSpace && bInFrontSpace && cInFrontSpace) {
                continue;
            }

            // All points invisible, discard.
            if (!aInFrontSpace && !bInFrontSpace && !cInFrontSpace) {
                keep = false;
                break;
            }

            // Two possible cases: either one inside, two outside; or two inside, one outside.
            float da = plane.distance(triangle.a.clipPosition);
            float db = plane.distance(triangle.b.clipPosition);
            float dc = plane.distance(triangle.c.clipPosition);

            // a inside; b,c outside;
            if (da >= 0 && db < 0 && dc < 0) {
                triangle.b = clipEdge(triangle.a, triangle.b, plane);
                triangle.c = clipEdge(triangle.a, triangle.c, plane);
                continue;
            }

            // b inside; a,c outside;
            if (db >= 0 && da < 0 && dc < 0) {
                triangle.a = clipEdge(triangle.a, triangle.b, plane);
                triangle.c = clipEdge(triangle.c, triangle.b, plane);
                continue;
            }

            // c inside, a,b outside
            if (dc >= 0 && da < 0 && db < 0) {
                triangle.a = clipEdge(triangle.c, triangle.a, plane);
                triangle.b = clipEdge(triangle.c, triangle.b, plane);
                continue;
            }

            if (da >= 0 && db >= 0) {
                VertexOut p = clipEdge(triangle.a, triangle.c, plane);
                VertexOut q = clipEdge(triangle.b, triangle.c, plane);
                // Truncate current triangle, add new one to the list.
                triangle.c = p;

                TrianglePrimitive t(p, triangle.b, q);
                if (colorClips) {
                    setColor(t, glm::vec4(1, 0, 1, 1));
                }
                triangles.push_back(t);
                continue;
            }

            if (da >= 0 && dc >= 0) {
                VertexOut p = clipEdge(triangle.a, triangle.b, plane);
                VertexOut q = clipEdge(triangle.c, triangle.b, plane);
                triangle.b = p;

                TrianglePrimitive t(p, q, triangle.c);
                if (colorClips) {
                    setColor(t, glm::vec4(1, 1, 0, 1));
                }
                triangles.push_back(t);
                continue;
            }

            if (db >= 0 && dc >= 0) {
                VertexOut p = clipEdge(triangle.b, triangle.a, plane);
                VertexOut q = clipEdge(triangle.c, triangle.a, plane);
                triangle.a = p;

                TrianglePrimitive t(p, triangle.c, q);
                if (colorClips) {
                    setColor(t, glm::vec4(0, 1, 1, 1));
                }
                triangles.push_back(t);
                continue;
            }
        }

        if (keep) {
            clipped.push_back(triangle);
        }
    }
    return clipped;
}

TrianglePrimitiveList Clipper::clipTrianglesToNdc(TrianglePrimitiveList clipspaceTriangles) const {
    using glm::vec4;
    using glm::dot;

    TrianglePrimitiveList clipped;

    // Note: OpenGL's clip space is LEFT-handed, see: https://stackoverflow.com/questions/4124041/is-opengl-coordinate-system-left-handed-or-right-handed
    std::vector<vec4> planes;
    /*
    planes.push_back(vec4(0,0,1,1)); // near plane
    planes.push_back(vec4(0,0,-1,1)); // far plane
    planes.push_back(vec4(0,1,0,1)); // bottom
    planes.push_back(vec4(0,-1,0,1)); // top
    */
    planes.push_back(vec4(1, 0, 0, 1)); // left
    //planes.push_back(vec4(-1, 0, 0, 1)); // right

    // safe-guard against adding too many clipped triangles.
    const size_t MAX_TRIS = 100;

    for (size_t i = 0; i < clipspaceTriangles.size() && i < MAX_TRIS; ++i) {
        TrianglePrimitive triangle = clipspaceTriangles[i];

        bool keepTriangle = true;
        for (const vec4& plane : planes) {
            //std::clog << "plane " << plane << std::endl;

            // Signed plane-vertex distances.
            float da = dot(triangle.a.clipPosition, plane);
            float db = dot(triangle.b.clipPosition, plane);
            float dc = dot(triangle.c.clipPosition, plane);

            float e = glm::epsilon<float>();
            if (da > -e || da < e) da = 0;
            if (db > -e || db < e) db = 0;
            if (dc > -e || dc > e) dc = 0;


            if (da < 0 && db < 0 && dc < 0) {
                //std::clog << "All outside -- discarding.\n";
                keepTriangle = false;
                break;
            } else if (da >= 0 && db >= 0 && dc >= 0) {
                //std::clog << "All inside -- continuing.\n";
                continue;
            } else {
                // a inside; b,c outside;
                if (da >= 0 && db < 0 && dc < 0) {
                    //std::clog << "Clip bc\n";
                    float tab = da / (da - db);
                    triangle.b = lerp(triangle.a, triangle.b, tab);
                    float tac = da / (da - dc);
                    triangle.c = lerp(triangle.a, triangle.c, tac);

                    triangle.setColor(vec4(1,0,0,1));
                    continue;
                }

                // b inside; a,c outside;
                if (db >= 0 && da < 0 && dc < 0) {
                    //std::clog << "Clip ac\n";
                    float tba = db / (db - da);
                    triangle.a = lerp(triangle.b, triangle.a, tba);
                    float tbc = db / (db - dc);
                    triangle.c = lerp(triangle.b, triangle.c, tbc);

                    triangle.setColor(vec4(0,1,0,1));
                    continue;
                }

                // c inside, a,b outside
                if (dc >= 0 && da < 0 && db < 0) {
                    //std::clog << "Clip ab\n";
                    float tca = dc / (dc - da);
                    triangle.a = lerp(triangle.c, triangle.a, tca);
                    float tcb = dc / (dc - db);
                    triangle.b = lerp(triangle.c, triangle.b, tcb);

                    triangle.setColor(vec4(0,0,1,1));
                    continue;
                }

                if (db >= 0 && dc >= 0) {
                    //std::clog << "Clip a\n";
                    float tab = da / (da - db);
                    float tac = da / (da - dc);

                    VertexOut p = lerp(triangle.a, triangle.b, tab);
                    VertexOut q = lerp(triangle.a, triangle.c, tac);
                    triangle.a = p;
                    triangle.setColor(vec4(0,1,1,1));

                    TrianglePrimitive t(p, triangle.c, q);
                    t.setColor(vec4(0,0.5,0.5,1));
                    clipspaceTriangles.push_back(t);
                    continue;
                }

                if (da >= 0 && dc >= 0) {
                    //std::clog << "Clip b\n";
                    float tba = db / (db - da);
                    float tbc = db / (db - dc);

                    VertexOut p = lerp(triangle.b, triangle.a, tba);
                    VertexOut q = lerp(triangle.b, triangle.c, tbc);
                    triangle.b = p;
                    triangle.setColor(vec4(1,0,1,1));

                    TrianglePrimitive t(p, q, triangle.c);
                    t.setColor(vec4(0.5,0,0.5,1));
                    clipspaceTriangles.push_back(t);
                    continue;
                }

                if (da >= 0 && db >= 0) {
                    //std::clog << "Clip c\n";
                    float tca = dc / (dc / da);
                    float tcb = dc / (dc / db);

                    VertexOut p = lerp(triangle.c, triangle.a, tca);
                    VertexOut q = lerp(triangle.c, triangle.b, tcb);
                    triangle.c = p;
                    triangle.setColor(vec4(1,1,0,1));

                    TrianglePrimitive t(p, triangle.b, q);
                    t.setColor(vec4(0.5,0.5,0,1));
                    clipspaceTriangles.push_back(t);
                    continue;
                }
            }
        }

        if (keepTriangle) {
            clipped.push_back(triangle);
        }
    }

    std::clog << "Clipped to " << clipped.size() << " visible tris.\n";

    return clipped;
}

}
