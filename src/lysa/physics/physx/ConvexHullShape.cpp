/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.resources.convex_hull_shape;

import lysa.global;

namespace lysa {

    void ConvexHullShape::createShape() {
        std::vector<physx::PxVec3> pxPoints;
        for (const auto &vertex : points) {
            pxPoints.push_back(physx::PxVec3{vertex.x, vertex.y, vertex.z});
        }
        auto convexDesc = physx::PxConvexMeshDesc{};
        convexDesc.points.count     = pxPoints.size();
        convexDesc.points.stride    = sizeof(physx::PxVec3);
        convexDesc.points.data      = pxPoints.data();
        convexDesc.flags            = physx::PxConvexFlag::eCOMPUTE_CONVEX;

        const physx::PxCookingParams params{physx::PxTolerancesScale()};
        physx::PxConvexMeshCookingResult::Enum result;
        physx::PxDefaultMemoryOutputStream blob;
        if (!PxCookConvexMesh(params, convexDesc, blob, &result)) {
            throw Exception("ConvexHullShape cooking failed");
        }
        physx::PxDefaultMemoryInputData input(blob.getData(), blob.getSize());
        geometry = std::make_unique<physx::PxConvexMeshGeometry>(getPhysx()->createConvexMesh(input));
    }

}
