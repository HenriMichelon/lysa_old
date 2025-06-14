/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
module lysa.resources.shape;

import lysa.application;
import lysa.global;
import lysa.nodes.mesh_instance;
import lysa.physics.engine;

namespace lysa {

    Shape::Shape(PhysicsMaterial* material, const std::wstring &resName):
        Resource{resName}{
        this->material = material ?
            material->duplicate():
            Application::getPhysicsEngine().createMaterial();
    }

    AABBShape::AABBShape(
        const Node &node,
        PhysicsMaterial* material,
        const std::wstring &resName ):
        Shape{material, resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance) {
            const auto& aabb = meshInstance->getMesh()->getAABB();
            const auto& extends = (aabb.max - aabb.min) * 0.5f;
            shapeSettings = new JPH::BoxShapeSettings(
                JPH::Vec3(extends.x, extends.y, extends.z),
                JPH::cDefaultConvexRadius,
                reinterpret_cast<JPH::PhysicsMaterial*>(this->material));
        } else {
            throw Exception("AABBShape : Node ", lysa::to_string(node.getName()), "does not have a MeshInstance child");
        }
    }

    BoxShape::BoxShape(
        const float3& extends,
        PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName}, extends
        {extends} {
        if (extends.x <= 0.2 || extends.y <= 0.2 || extends.z <= 0.2) {
            throw Exception("Invalid extends for BoxShape", lysa::to_string(resName), " : extends must be greater than 0.2");
        }
        shapeSettings = new JPH::BoxShapeSettings(
            JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2),
            JPH::cDefaultConvexRadius,
            reinterpret_cast<JPH::PhysicsMaterial*>(this->material));
    }

    std::shared_ptr<Resource> BoxShape::duplicate() const {
        auto dup = std::make_shared<BoxShape>(extends, material, getName());
        dup->shapeSettings = new JPH::BoxShapeSettings(
            JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2),
            JPH::cDefaultConvexRadius,
            reinterpret_cast<JPH::PhysicsMaterial*>(this->material));
        return dup;
    }

    SphereShape::SphereShape(
        const float radius,
        PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName} {
        shapeSettings = new JPH::SphereShapeSettings(
            radius,
            reinterpret_cast<JPH::PhysicsMaterial*>(this->material));
    }

}
