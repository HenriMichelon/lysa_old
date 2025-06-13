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

import lysa.global;
import lysa.nodes.mesh_instance;

namespace lysa {

    AABBShape::AABBShape(const Node &node, const std::wstring &resName ): Shape{resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance) {
            const auto& aabb = meshInstance->getMesh()->getAABB();
            const auto& extends = (aabb.max - aabb.min) * 0.5f;
            shapeHandle = new JPH::BoxShapeSettings(JPH::Vec3(extends.x, extends.y, extends.z));
        } else {
            throw Exception("AABBShape : Node ", lysa::to_string(node.getName()), "does not have a MeshInstance child");
        }
    }

    BoxShape::BoxShape(const float3& extends, const std::wstring &resName):
        Shape{resName}, extends
        {extends} {
        if (extends.x <= 0.2 || extends.y <= 0.2 || extends.z <= 0.2) {
            throw Exception("Invalid extends for BoxShape", lysa::to_string(resName), " : extends must be greater than 0.2");
        }
        shapeHandle = new JPH::BoxShapeSettings(JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2));
    }

    std::shared_ptr<Resource> BoxShape::duplicate() const {
        auto dup = std::make_shared<BoxShape>(extends, getName());
        dup->shapeHandle = new JPH::BoxShapeSettings(JPH::Vec3(extends.x / 2, extends.y / 2, extends.z / 2));
        return dup;
    }

    SphereShape::SphereShape(const float radius, const std::wstring &resName):
        Shape{resName} {
        shapeHandle = new JPH::SphereShapeSettings(radius);
    }

    CylinderShape::CylinderShape(const float radius, const float height, const std::wstring &resName):
        Shape{resName} {
        shapeHandle = new JPH::CylinderShapeSettings(height / 2.0f, radius);
    }
}
