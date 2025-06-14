/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.collision_area;

import lysa.global;
import lysa.nodes.node;

namespace lysa {

    void CollisionArea::setShape(const std::shared_ptr<Shape> &shape) {
        if (this->shape) {
            releaseResources();
        }
        this->shape = shape;
        const auto position = getPositionGlobal();
        const auto quat = getRotationGlobal();
        // JPH::BodyCreationSettings settings{
        //         reinterpret_cast<JPH::ShapeSettings*>(shape->getShapeHandle()),
        //         JPH::RVec3{position.x, position.y, position.z},
        //         JPH::Quat{quat.x, quat.y, quat.z, quat.w},
        //         JPH::EMotionType::Dynamic,
        //         collisionLayer,
        // };
        // settings.mIsSensor                     = true;
        // settings.mUseManifoldReduction         = true;
        // settings.mOverrideMassProperties       = JPH::EOverrideMassProperties::MassAndInertiaProvided;
        // settings.mMassPropertiesOverride       = JPH::MassProperties{.mMass = 1.0f,.mInertia = JPH::Mat44::sIdentity()};
        // // settings.mCollideKinematicVsNonDynamic = true;
        // settings.mGravityFactor                = 0.0f;
        // const auto body = bodyInterface->CreateBody(settings);
        // setBodyId(body->GetID());
    }

}
