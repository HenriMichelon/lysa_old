/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
module lysa.resources.convex_hull_shape;

namespace lysa {

    void ConvexHullShape::createShape() {
        JPH::Array<JPH::Vec3> jphPoints;
        for (const auto &vertex : points) {
            jphPoints.push_back(JPH::Vec3{vertex.x, vertex.y, vertex.z});
        }
        shapeHandle = new JPH::ConvexHullShapeSettings(jphPoints);
    }

}
