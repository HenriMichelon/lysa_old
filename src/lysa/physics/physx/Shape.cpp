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

    AABBShape::AABBShape(const Node &node, const std::wstring &resName ): Shape{resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance) {
            const auto& aabb = meshInstance->getMesh()->getAABB();
            const auto& extends = (aabb.max - aabb.min) * 0.5f;
            const auto physx = dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine()).getPhysics();
            shapeHandle = physx->createShape(
                physx::PxBoxGeometry(extends.x / 2, extends.y / 2, extends.z / 2),
                *physx->createMaterial(0.5f, 0.5f, 0.0f));
        } else {
            throw Exception("AABBShape : Node ", lysa::to_string(node.getName()), "does not have a MeshInstance child");
        }
    }

    BoxShape::BoxShape(const float3& extends, const std::wstring &resName):
        Shape{resName}, extends{extends} {
        const auto physx = dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine()).getPhysics();
        shapeHandle = physx->createShape(
            physx::PxBoxGeometry(extends.x / 2, extends.y / 2, extends.z / 2),
            *physx->createMaterial(0.5f, 0.5f, 0.0f));
    }

    std::shared_ptr<Resource> BoxShape::duplicate() const {
        auto dup = std::make_shared<BoxShape>(extends, getName());
        const auto physx = dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine()).getPhysics();
        dup->shapeHandle = physx->createShape(
            physx::PxBoxGeometry(extends.x / 2, extends.y / 2, extends.z / 2),
            *physx->createMaterial(0.5f, 0.5f, 0.0f));
        return dup;
    }

    SphereShape::SphereShape(const float radius, const std::wstring &resName):
        Shape{resName} {
        const auto physx = dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine()).getPhysics();
        shapeHandle = physx->createShape(
            physx::PxSphereGeometry (radius),
            *physx->createMaterial(0.5f, 0.5f, 0.0f));
    }

}
