/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
module lysa.resources.static_compound_shape;

namespace lysa {

    StaticCompoundShape::StaticCompoundShape(const std::vector<SubShape> &subshapes, const std::string &resName) :
        Shape{nullptr, resName} {
        const auto settings = new JPH::StaticCompoundShapeSettings();
        for (const auto &subshape : subshapes) {
            const auto quat = quaternion{subshape.rotation};
            settings->AddShape(JPH::Vec3{subshape.position.x, subshape.position.y, subshape.position.z},
                               JPH::Quat{quat.x, quat.y, quat.z, quat.w},
                               subshape.shape->getShapeSettings());
        }
        shapeSettings = settings;
    }

}
