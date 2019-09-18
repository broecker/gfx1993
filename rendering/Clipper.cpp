#include "Clipper.h"

namespace render {

Clipper::Clipper(const Clipper::Plane &plane) {
    planes.push_back(plane);
}

Clipper::Clipper(const std::vector<Plane> &clipPlanes) : planes(clipPlanes) {}

Clipper::Clipper() {
    using glm::vec4;

    // The six clip planes in NDC.
    planes.push_back(Plane(vec4(1,0,0,1)));
    planes.push_back(Plane(vec4(-1,0,0,1)));
    planes.push_back(Plane(vec4(0,1,0,1)));
    planes.push_back(Plane(vec4(0,-1,0,1)));
    planes.push_back(Plane(vec4(0,0,1,1)));
    planes.push_back(Plane(vec4(0,0,-1,1)));
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

LinePrimitiveList Clipper::clipLines(const render::LinePrimitiveList &lines) const {
    LinePrimitiveList clipped; clipped.reserve(lines.size());

    // Make a copy while iterating, as we might need to change the line's endpoints.
    for (LinePrimitive line : lines) {

        bool keep = true;
        for (const Plane& p : planes) {
            bool aInFrontSpace = p.inFrontSpace(line.a.clipPosition);
            bool bInFrontSpace = p.inFrontSpace(line.b.clipPosition);

            // Both ends of the line are in front of the plane -- keep going.
            if (aInFrontSpace && bInFrontSpace) {
                continue;
            }

            // Both ends of the line are behind the plane -- discard.
            if (!aInFrontSpace && !bInFrontSpace) {
                keep = false;
                break;
            }

            // Otherwise, calculate intersection and clip line.
            float d0 = p.distance(line.a.clipPosition);
            float d1 = p.distance(line.b.clipPosition);

            // Calculate intersection;
            float t = d0 / (d0 - d1);

            // a in front, b behind plane.
            if (d0 >= 0 && d1 < 0) {
                line.b = lerp(line.a, line.b, t);
            }

            // b in front, a behind plane.
            if (d0 < 0 && d1 >= 0) {
                line.a = lerp(line.a, line.b, t);
            }
        }
        if (keep) {
            clipped.push_back(line);
        }
    }
    return clipped;
}

}
