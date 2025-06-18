/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.resources.mesh_shape;

import lysa.global;

namespace lysa {

    std::unique_ptr<physx::PxGeometry> MeshShape::getGeometry(const float3& scale) const {
        const auto & vertices  = meshInstance->getMesh()->getVertices();
        const auto& indices = meshInstance->getMesh()->getIndices();
        const auto &transform = meshInstance->getTransform();

        std::vector<physx::PxVec3> pxVertices;
        pxVertices.reserve(vertices.size());
        for (const auto& v : vertices) {
            auto point = mul(float4{v.position, 1.0f}, transform);
            pxVertices.emplace_back(point.x, point.y, point.z);
        }

        std::vector<physx::PxU32> pxIndices;
        pxIndices.reserve(indices.size());
        for (uint32_t index : indices) {
            pxIndices.push_back(index);
        }

        auto meshDesc = physx::PxTriangleMeshDesc {};
        meshDesc.points.count     = static_cast<physx::PxU32>(pxVertices.size());
        meshDesc.points.stride    = sizeof(physx::PxVec3);
        meshDesc.points.data      = pxVertices.data();
        meshDesc.triangles.count  = static_cast<physx::PxU32>(pxIndices.size() / 3);
        meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
        meshDesc.triangles.data   = pxIndices.data();
        meshDesc.flags            = physx::PxMeshFlag::e16_BIT_INDICES;

        const physx::PxCookingParams cookingParams{physx::PxTolerancesScale{}};
        physx::PxDefaultMemoryOutputStream stream;
        if (!PxCookTriangleMesh(cookingParams, meshDesc, stream)) {
            throw Exception("MeshShape Failed to cook triangle mesh");
        }
        physx::PxDefaultMemoryInputData input(stream.getData(), stream.getSize());
        return std::make_unique<physx::PxTriangleMeshGeometry>(getPhysx()->createTriangleMesh(input));
    }

}
