/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.resources.shape;

import lysa.application;
import lysa.global;
import lysa.nodes.mesh_instance;
import lysa.physics.physx.engine;

namespace lysa {

    AABBShape::AABBShape(
        const Node &node,
        const PhysicsMaterial* material,
        const std::wstring &resName ):
        Shape{material, resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance) {
            const auto& aabb = meshInstance->getMesh()->getAABB();
            extends = (aabb.max - aabb.min) * 0.5f;
        } else {
            throw Exception("AABBShape : Node ", lysa::to_string(node.getName()), "does not have a MeshInstance child");
        }
    }

    std::unique_ptr<physx::PxGeometry> AABBShape::getGeometry(const float3& scale) const {
        const auto scaledExtents = extends * scale;
        return std::make_unique<physx::PxBoxGeometry>(scaledExtents.x / 2, scaledExtents.y / 2, scaledExtents.z / 2);
    }

    BoxShape::BoxShape(
        const float3& extends,
        PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName}, extends{extends} {
    }

    std::unique_ptr<physx::PxGeometry> BoxShape::getGeometry(const float3& scale) const {
        const auto scaledExtents = extends * scale;
        return std::make_unique<physx::PxBoxGeometry>(scaledExtents.x / 2, scaledExtents.y / 2, scaledExtents.z / 2);
    }

    std::shared_ptr<Resource> BoxShape::duplicate() const {
        return std::make_shared<BoxShape>(extends, material, getName());
    }

    SphereShape::SphereShape(
        const float radius,
        const PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName},
        radius{radius} {
    }

    std::unique_ptr<physx::PxGeometry> SphereShape::getGeometry(const float3& scale) const {
        return std::make_unique<physx::PxSphereGeometry>(radius * scale.x);
    }

}
