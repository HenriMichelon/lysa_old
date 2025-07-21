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
module lysa.nodes.kinematic_body;

import lysa.resources.shape;

namespace lysa {

    KinematicBody::KinematicBody(const std::shared_ptr<Shape>& shape,
                                 const collision_layer  layer,
                                 const std::string& name):
        PhysicsBody(shape,
                    layer,
                    JPH::EActivation::Activate,
                    JPH::EMotionType::Kinematic,
                    name,
                    KINEMATIC_BODY) {
    }

    KinematicBody::KinematicBody(const std::string& name):
       PhysicsBody(0,
                   JPH::EActivation::Activate,
                   JPH::EMotionType::Kinematic,
                   name,
                   KINEMATIC_BODY) {
    }

}
