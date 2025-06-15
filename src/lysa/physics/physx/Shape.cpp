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
            const auto& extends = (aabb.max - aabb.min) * 0.5f;
            geometry = std::make_unique<physx::PxBoxGeometry>(extends.x / 2, extends.y / 2, extends.z / 2);
        } else {
            throw Exception("AABBShape : Node ", lysa::to_string(node.getName()), "does not have a MeshInstance child");
        }
    }

    BoxShape::BoxShape(
        const float3& extends,
        PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName}, extends{extends} {
        geometry = std::make_unique<physx::PxBoxGeometry>(extends.x / 2, extends.y / 2, extends.z / 2);
    }

    std::shared_ptr<Resource> BoxShape::duplicate() const {
        auto dup = std::make_shared<BoxShape>(extends, material, getName());
        dup->geometry = std::make_unique<physx::PxBoxGeometry>(extends.x / 2, extends.y / 2, extends.z / 2);
        return dup;
    }

    SphereShape::SphereShape(
        const float radius,
        const PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName} {
        geometry = std::make_unique<physx::PxSphereGeometry>(radius);
    }

}
