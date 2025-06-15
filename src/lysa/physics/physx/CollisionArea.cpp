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
        const auto& physx = getPhysx();

        const auto position = getPositionGlobal();
        const auto quat = getRotationGlobal();
        const physx::PxTransform transform{
            physx::PxVec3(position.x, position.y, position.z),
            physx::PxQuat(quat.x, quat.y, quat.z, quat.w)};
        const auto actor = physx->createRigidStatic(transform);

        const physx::PxShapeFlags shapeFlags =
            physx::PxShapeFlag::eSCENE_QUERY_SHAPE |
            physx::PxShapeFlag::eTRIGGER_SHAPE;
        const auto pxShape = physx->createShape(shape->getGeometry(), shape->getMaterial(), true, shapeFlags);
        shapes.push_back(pxShape);
        actor->attachShape(*pxShape);
        setActor(actor);
    }

}
