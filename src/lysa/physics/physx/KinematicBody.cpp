/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.kinematic_body;

import lysa.resources.shape;

namespace lysa {

    KinematicBody::KinematicBody(const std::shared_ptr<Shape>& shape,
                                 const collision_layer  layer,
                                 const std::string& name):
        PhysicsBody(shape,
                    layer,
                    physx::PxActorType::eRIGID_DYNAMIC,
                    name,
                    KINEMATIC_BODY) {
    }

    KinematicBody::KinematicBody(const std::string& name):
       PhysicsBody(0,
                   physx::PxActorType::eRIGID_DYNAMIC,
                   name,
                   KINEMATIC_BODY) {
    }

}
