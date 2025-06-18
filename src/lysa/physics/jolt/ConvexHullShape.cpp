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

import lysa.application;

namespace lysa {

    JPH::ShapeSettings* ConvexHullShape::getShapeSettings() {
        std::list<float3> points;
        const auto &transform = meshInstance->getTransform();
        for (const auto &vertex : meshInstance->getMesh()->getVertices()) {
            auto point = mul(float4{vertex.position, 1.0f}, transform);
            points.push_back(point.xyz);
        }
        JPH::Array<JPH::Vec3> jphPoints;
        for (const auto &vertex : points) {
            jphPoints.push_back(JPH::Vec3{vertex.x, vertex.y, vertex.z});
        }
        shapeSettings = new JPH::ConvexHullShapeSettings(
            jphPoints,
            JPH::cDefaultConvexRadius,
            reinterpret_cast<JPH::PhysicsMaterial*>(material));
        return shapeSettings;
    }

}
