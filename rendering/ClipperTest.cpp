#include <string>
#include <cassert>

#include "Clipper.h"

using namespace render;
using glm::vec2;
using glm::vec3;
using glm::vec4;

PointPrimitive makePoint(const vec3& p) {
    VertexOut vout;
    vout.clipPosition = vec4(p, 1);
    return PointPrimitive(vout);
}

LinePrimitive makeLine(const vec3& a, const vec3& b) {
    VertexOut va;
    va.clipPosition = vec4(a, 1);
    va.texcoord = vec2(0);

    VertexOut vb;
    vb.clipPosition = vec4(b, 1);
    vb.texcoord = vec2(1);

    return LinePrimitive(va, vb);
}

int testPlaneDistances()
{
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec4(0, 1, 0, 0));

    assert(plane.distance(vec3(0,0,0)) == 0);
    assert(plane.distance(vec3(2,0,5)) == 0);
    assert(plane.distance(vec3(0,1,0)) == 1);
    assert(plane.distance(vec3(0,-1,0)) == -1);

    return 0;
}

int testPlaneCoplanar() {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec4(0, 1, 0, 0));

    assert(plane.inFrontSpace(vec3(0,0,0)));
    return 0;
}

int testPlaneBackSpace() {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec4(0, 1, 0, 0));

    assert(!plane.inFrontSpace(vec3(0,-2,0)));
    return 0;
}

int testPlaneFrontSpace() {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec4(0, 1, 0, 0));

    assert(plane.inFrontSpace(vec3(0,2,0)));
    return 0;
};

int testClipperPointSinglePlane() {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec4(0,1,0,0)));

    PointPrimitiveList points;
    points.push_back(makePoint(vec3(0, 2, 0))); // In front space.
    points.push_back(makePoint(vec3(0, -2, 0))); // In back space.

    PointPrimitiveList clipped = clipper.clipPoints(points);

    assert(clipped.size() == 1);
    assert(clipped[0].p.clipPosition == vec4(0,2,0,1));
    return 0;
}

int testClipperPointSinglePlane2() {
    // XZ plane, normal pointing down at Y+2
    Clipper clipper(Clipper::Plane(vec4(0,-1,0,2)));

    PointPrimitiveList points;
    for (int i = 0; i <= 10; ++i) {
        points.push_back(makePoint(vec3(0, i, 0)));
    }

    PointPrimitiveList clipped = clipper.clipPoints(points);

    assert(clipped.size() == 3);
    assert(clipped[0].p.clipPosition.y == 0);
    assert(clipped[1].p.clipPosition.y == 1);
    assert(clipped[2].p.clipPosition.y == 2);
    return 0;
}

int testClipperPointSinglePlane3() {
    // YZ plane at X+2 with normal at +X
    Clipper clipper(Clipper::Plane(vec4(1,0,0,-2)));

    PointPrimitiveList points;
    for (int x = -4; x <= 4; ++x) {
        points.push_back(makePoint(vec3(x, 0, 0)));
    }

    PointPrimitiveList clipped = clipper.clipPoints(points);

    assert(clipped.size() == 3);
    assert(clipped[0].p.clipPosition.x == 2);
    assert(clipped[1].p.clipPosition.x == 3);
    assert(clipped[2].p.clipPosition.x == 4);
    return 0;
}

int testClipperPointsBetweenTwoPlanes() {
    std::vector<Clipper::Plane> planes;
    // Bottom plane, at origin pointing up.
    planes.push_back(Clipper::Plane(vec4(0,1,0,0)));
    // Top plane, pointing down, 1 unit high.
    planes.push_back(Clipper::Plane(vec4(0,-1,0,1)));
    Clipper clipper(planes);

    PointPrimitiveList points;
    points.push_back(makePoint(vec3(0, 0.5, 0))); // Inside
    points.push_back(makePoint(vec3(0, 2, 0))); // Too high
    points.push_back(makePoint(vec3(0, -2, 0))); // Too low
    points.push_back(makePoint(vec3(0, 0, 0))); // Coplanar with bottom plane.

    PointPrimitiveList clipped = clipper.clipPoints(points);

    assert(clipped.size() == 2);
    assert(clipped[0].p.clipPosition == vec4(0, 0.5, 0, 1));
    assert(clipped[1].p.clipPosition == vec4(0, 0, 0, 1));

    return 0;
}

int testClipperLineSinglePlane() {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec4(0,1,0,0)));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(0, 1, 0), vec3(0, -1, 0))); // Single line, top to bottom.
    lines.push_back(makeLine(vec3(0, -1, 0), vec3(0, 1, 0))); // Single bottom, to top.

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.size() == 2);

    assert(clipped[0].a.clipPosition == vec4(0, 1, 0, 1)); // First endpoint is unchanged.
    assert(clipped[0].a.texcoord == vec2(0));
    assert(clipped[0].b.clipPosition == vec4(0, 0, 0, 1)); // Second endpoint got moved.
    assert(clipped[0].b.texcoord == vec2(0.5)); // Second parameters got interpolated.

    assert(clipped[1].a.clipPosition == vec4(0, 0, 0, 1)); // First endpoint moved to the plane.
    assert(clipped[1].a.texcoord == vec2(0.5));
    assert(clipped[1].b.clipPosition == vec4(0, 1, 0, 1)); // Second endpoint is unchanged.
    assert(clipped[1].b.texcoord == vec2(1));

    return 0;
}


int testClipperLineSinglePlaneDiscardsInBack() {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec4(0,1,0,0)));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, -2, 0), vec3(5, -3, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.empty());
    return 0;
}

int testClipperLineSinglePlaneKeepsInFront() {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec4(0,1,0,0)));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, 1, 0), vec3(5, 0, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.size() == 1);
    // Endpoints unchanged.
    assert(clipped[0].a.clipPosition == vec4(-2, 1, 0, 1));
    assert(clipped[0].a.texcoord == vec2(0));
    assert(clipped[0].b.clipPosition == vec4(5, 0, 2, 1));
    assert(clipped[0].b.texcoord == vec2(1));
    return 0;
}

int testClipperLineMultiplePlanes() {
    std::vector<Clipper::Plane> planes;
    // Bottom XZ plane at Y-1; normal pointing up.
    planes.push_back(Clipper::Plane(vec4(0,1,0,1)));
    // Top XZ plane at Y+1; normal pointing down.
    planes.push_back(Clipper::Plane(vec4(0,-1,0,1)));
    Clipper clipper(planes);

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(0, 2, 0), vec3(0, -2, 0))); // Single line, top to bottom.
    lines.push_back(makeLine(vec3(0, -2, 0), vec3(0, 2, 0))); // Single bottom, to top.

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.size() == 2);

    assert(clipped[0].a.clipPosition == vec4(0,1,0,1));
    assert(clipped[0].a.texcoord == vec2(0.25));
    assert(clipped[0].b.clipPosition == vec4(0,-1,0,1));
    assert(clipped[0].b.texcoord == vec2(0.75));

    assert(clipped[1].a.clipPosition == vec4(0,-1,0,1));
    assert(clipped[1].a.texcoord == vec2(0.25));
    assert(clipped[1].b.clipPosition == vec4(0,1,0,1));
    assert(clipped[1].b.texcoord == vec2(0.75));

    return 0;
}

// Two planes at right angles.
int testClipperLineMultiplePlanes2() {
    std::vector<Clipper::Plane> planes;
    // Bottom XZ plane at origin; normal pointing up.
    planes.push_back(Clipper::Plane(vec4(0,1,0,0)));
    // Right YZ plane at X+2; normal point at X-1
    planes.push_back(Clipper::Plane(vec4(-1,0,0,2)));
    Clipper clipper(planes);

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, -2, 0), vec3(4, 4, 0)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.size() == 1);
    assert(clipped[0].a.clipPosition == vec4(0,0,0,1));
    assert(clipped[0].b.clipPosition == vec4(2,2,0,1));

    return 0;
}

int testClipperLineCoplanarPlane()
{
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec4(0,1,0,0)));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(5, 0, 3), vec3(-5, 0, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.size() == 1);
    // Both endpoints are unchanged.
    assert(clipped[0].a.clipPosition == vec4(5,0,3,1));
    assert(clipped[0].a.texcoord == vec2(0));
    assert(clipped[0].b.clipPosition == vec4(-5,0,2,1));
    assert(clipped[0].b.texcoord == vec2(1));

    return 0;
}

int testClipperCreateNdcPlanes()
{
    Clipper clipper;

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, 0, 0), vec3(2, 0, 0)));
    lines.push_back(makeLine(vec3(0, -2, 0), vec3(0, 2, 0)));
    lines.push_back(makeLine(vec3(0, 0, -2), vec3(0, 0, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    assert(clipped.size() == 3);
    assert(clipped[0].a.clipPosition == vec4(-1, 0, 0, 1));
    assert(clipped[0].b.clipPosition == vec4(1, 0, 0, 1));
    assert(clipped[1].a.clipPosition == vec4(0, -1, 0, 1));
    assert(clipped[1].b.clipPosition == vec4(0, 1, 0, 1));
    assert(clipped[2].a.clipPosition == vec4(0, 0, -1, 1));
    assert(clipped[2].a.clipPosition == vec4(0, 0, 1, 1));

    return 0;
}

int main(int argc, const char** argv) {
    const std::string test(argv[1]);

    if (test == "plane-distances") {
        return testPlaneDistances();
    }

    if (test == "plane-frontspace") {
        return testPlaneFrontSpace();
    }

    if (test == "plane-backspace") {
        return testPlaneBackSpace();
    }

    if (test == "plane-coplanar") {
        return testPlaneCoplanar();
    }

    if (test == "clipper-point-single-plane") {
        testClipperPointSinglePlane();
        testClipperPointSinglePlane2();
        testClipperPointSinglePlane3();
        return 0;
    }

    if (test == "clipper-points-multiple-planes") {
        return testClipperPointsBetweenTwoPlanes();
    }

    if (test == "clipper-line-single-keep") {
        return testClipperLineSinglePlaneKeepsInFront();
    }

    if (test == "clipper-line-single-discard") {
        return testClipperLineSinglePlaneDiscardsInBack();
    }

    if (test == "clipper-line-single-plane") {
        return testClipperLineSinglePlane();
    }

    if (test == "clipper-line-multiple-planes") {
        testClipperLineMultiplePlanes();
        testClipperLineMultiplePlanes2();
        return 0;
    }

    if (test == "clipper-line-coplanar-plane") {
        return testClipperLineCoplanarPlane();
    }

    if (test == "clipper-creates-ndc-planes") {
        return testClipperCreateNdcPlanes();
    }

    return 0;
}


