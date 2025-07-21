/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/EActivation.h>
module lysa.nodes.static_body;

import lysa.nodes.node;

namespace lysa {

    StaticBody::StaticBody(const std::shared_ptr<Shape>& shape,
                           const collision_layer layer,
                           const std::string& name):
        PhysicsBody(shape,
                    layer,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name,
                    STATIC_BODY) {
    }

    StaticBody::StaticBody(const collision_layer layer,
                           const std::string& name):
        PhysicsBody(layer,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name,
                    STATIC_BODY) {
    }

    StaticBody::StaticBody(const std::string& name):
        PhysicsBody(0,
                    JPH::EActivation::DontActivate,
                    JPH::EMotionType::Static,
                    name,
                    STATIC_BODY) {
    }

}
