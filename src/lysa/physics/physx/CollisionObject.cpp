/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.nodes.collision_object;

import lysa.global;
import lysa.viewport;
import lysa.physics.physx.engine;

namespace lysa {

    CollisionObject::CollisionObject(
        const std::shared_ptr<Shape>& shape,
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        shape{shape},
        physX{dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine())} {
    }

    CollisionObject::CollisionObject(
        const uint32 layer,
        const std::wstring& name,
        const Type type):
        Node{name, type},
        collisionLayer{layer},
        physX{dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine())} {
    }

    CollisionObject::CollisionObject(const CollisionObject& other):
        Node{other.getName(), other.getType()},
        collisionLayer{other.collisionLayer},
        shape{other.shape},
        physX{dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine())} {
    }

    physx::PxScene* CollisionObject::getPxScene() const {
        if (getViewport()) {
            return dynamic_cast<PhysXPhysicsScene&>(getViewport()->getPhysicsScene()).getScene();
        }
        return nullptr;
    }

    void CollisionObject::releaseResources() {
        if (actor && scene) {
            scene->removeActor(*actor);
            actor->release();
            actor = nullptr;
        }
    }

    void CollisionObject::setCollisionLayer(const collision_layer layer) {
        collisionLayer = layer;
        if (actor) {
            physx::PxFilterData filterData;
            filterData.word0 = collisionLayer;
            for (const auto& pxShape : shapes) {
                pxShape->setQueryFilterData(filterData);
                pxShape->setSimulationFilterData(filterData);
            }
        }
    }

    void CollisionObject::setActor(physx::PxRigidActor *actor) {
        this->actor = actor;
        this->actor->userData = reinterpret_cast<void *>(this);
    }

    bool CollisionObject::wereInContact(const CollisionObject *obj) const {
        if (!actor || !scene) { return false; }
        throw Exception("Not implemented");
    }

    void CollisionObject::setPositionAndRotation() {
        if (updating || !actor || !scene) { return; }
        const auto pos = getPositionGlobal();
        const auto quat = normalize(getRotationGlobal());
        physx::PxTransform t(
            physx::PxVec3(pos.x, pos.y, pos.z),
            physx::PxQuat(quat.x, quat.y, quat.z, quat.w));
        actor->setGlobalPose(t);
    }

    void CollisionObject::scale(const float scale) {
        Node::scale(scale);
        if (!actor || !scene) { return; }
    }

    void CollisionObject::process(const float alpha) {
        Node::process(alpha);
        if (!actor || !scene) { return; }
        updating = true;
        const physx::PxTransform &pose = actor->getGlobalPose();
        setPositionGlobal(float3(pose.p.x, pose.p.y, pose.p.z));
        setRotationGlobal(quaternion(pose.q.x, pose.q.y, pose.q.z, pose.q.w));
        updating = false;
    }

    void CollisionObject::attachToViewport(Viewport* viewport) {
        Node::attachToViewport(viewport);
        scene = getPxScene();
    }

    void CollisionObject::enterScene() {
        if (isProcessed() && actor && scene && isVisible()) {
            scene->addActor(*actor);
            setPositionAndRotation();
        }
        Node::enterScene();
    }

    void CollisionObject::exitScene() {
        if (actor && scene) {
            scene->removeActor(*actor);
        }
        Node::exitScene();
    }

    void CollisionObject::pause() {
        if (actor && scene) {
            scene->removeActor(*actor);
        }
        Node::pause();
    }

    void CollisionObject::resume() {
        if (isProcessed() && actor) {
            if (isVisible() && scene) {
                scene->addActor(*actor);
                setPositionAndRotation();
            }
        }
        Node::resume();
    }

    void CollisionObject::setVisible(const bool visible) {
        if (actor && scene && visible != this->isVisible()) {
            if (isVisible()) {
                scene->addActor(*actor);
                setPositionAndRotation();
            } else {
                scene->removeActor(*actor);
            }
        }
        Node::setVisible(visible);
    }

}
