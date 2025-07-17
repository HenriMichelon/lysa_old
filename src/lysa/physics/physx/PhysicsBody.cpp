/*
 * Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.physics_body;

import lysa.global;
import lysa.log;
import lysa.viewport;
import lysa.resources.static_compound_shape;
import lysa.physics.physx.engine;

namespace lysa {

    PhysicsBody::PhysicsBody(
        const std::shared_ptr<Shape>& shape,
        const collision_layer layer,
        const physx::PxActorType::Enum actorType,
        const std::wstring& name,
        const Type type):
        CollisionObject{shape, layer, name, type},
        actorType{actorType} {
    }

    PhysicsBody::PhysicsBody(
        const collision_layer layer,
        const physx::PxActorType::Enum actorType,
        const std::wstring& name,
        const Type type):
        CollisionObject{layer, name, type},
        actorType{actorType} {
    }

    void preCreateBody(const std::shared_ptr<Shape> &shape) {
        this->shape = shape;
    }

    void PhysicsBody::createBody(const std::shared_ptr<Shape> &shape) {
        if (this->shape) {
            releaseResources();
        }
        this->shape = shape;
        const auto& physx = physX.getPhysics();
        const auto debug = getViewport()->getConfiguration().debugConfig.enabled;
        const auto &position = getPositionGlobal();
        const auto &quat = normalize(getRotationGlobal());
        const physx::PxTransform transform{
            physx::PxVec3(position.x, position.y, position.z),
            physx::PxQuat(quat.x, quat.y, quat.z, quat.w)};
        if (actorType == physx::PxActorType::eRIGID_DYNAMIC) {
            setActor(physx->createRigidDynamic(transform));
        } else {
            setActor(physx->createRigidStatic(transform));
        }
        if (debug) {
            actor->setActorFlag(physx::PxActorFlag::eVISUALIZATION, 1.0f);
        }
        createShape();
    }

    void PhysicsBody::createShape() {
        for (const auto& pxShape : shapes) {
            actor->detachShape(*pxShape);
            pxShape->release();
        }
        shapes.clear();
        const auto debug = getViewport()->getConfiguration().debugConfig.enabled;
        const auto& physx = physX.getPhysics();

        physx::PxFilterData filterData;
        filterData.word0 = collisionLayer;
        if (const auto& compound = std::dynamic_pointer_cast<StaticCompoundShape>(shape)) {
            for (const auto& subshape : compound->getSubShapes()) {
                auto pxShape = physx->createShape(*subshape.shape->getGeometry(getScale()), subshape.shape->getMaterial(), true);
                if (debug) {
                    pxShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, 1.0f);
                }
                pxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
                pxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
                const auto &localPos = subshape.position;
                const auto &localQuat = normalize(subshape.rotation);
                const physx::PxTransform localPose{
                    physx::PxVec3(localPos.x, localPos.y, localPos.z),
                    physx::PxQuat(localQuat.x, localQuat.y, localQuat.z, localQuat.w)};
                pxShape->setLocalPose(localPose);
                pxShape->setQueryFilterData(filterData);
                pxShape->setSimulationFilterData(filterData);
                shapes.push_back(pxShape);
                actor->attachShape(*pxShape);
            }
        } else {
            const auto pxShape = physx->createShape(*shape->getGeometry(getScale()), shape->getMaterial(), true);
            if (debug) {
                pxShape->setFlag(physx::PxShapeFlag::eVISUALIZATION, 1.0f);
            }
            pxShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            pxShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            pxShape->setQueryFilterData(filterData);
            pxShape->setSimulationFilterData(filterData);
            shapes.push_back(pxShape);
            actor->attachShape(*pxShape);
        }
    }

}
