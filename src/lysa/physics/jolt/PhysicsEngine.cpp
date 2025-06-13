/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
module lysa.physics.jolt.engine;

import lysa.application;
import lysa.global;
import lysa.nodes.collision_object;
import lysa.nodes.node;

namespace lysa {

    JPH::ValidateResult	ContactListener::OnContactValidate(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        JPH::RVec3Arg inBaseOffset,
        const JPH::CollideShapeResult &inCollisionResult) {
        const auto node1 = reinterpret_cast<CollisionObject*>(inBody1.GetUserData());
        const auto node2 = reinterpret_cast<CollisionObject*>(inBody2.GetUserData());
        assert([&]{ return node1 && node2;}, "physics body not associated with a node");
        return (node1->isProcessed() && node2->isProcessed())  ?
            JPH::ValidateResult::AcceptAllContactsForThisBodyPair :
            JPH::ValidateResult::RejectAllContactsForThisBodyPair;
    }

    void ContactListener::OnContactAdded(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings) {
        emit(CollisionObject::on_collision_starts, inBody1, inBody2, inManifold);
    }

    void ContactListener::OnContactPersisted(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings) {
        emit(CollisionObject::on_collision_persists, inBody1, inBody2, inManifold);
    }

    void ContactListener::emit(
        const Signal::signal &signal,
        const JPH::Body &body1,
        const JPH::Body &body2,
        const JPH::ContactManifold &inManifold) const {
        const auto node1 = reinterpret_cast<CollisionObject*>(body1.GetUserData());
        const auto node2 = reinterpret_cast<CollisionObject*>(body2.GetUserData());
        assert([&]{ return node1 && node2; }, "physics body not associated with a node");
        const auto normal = float3{
            inManifold.mWorldSpaceNormal.GetX(),
            inManifold.mWorldSpaceNormal.GetY(),
            inManifold.mWorldSpaceNormal.GetZ()};
        if (node1->getType() != Node::CHARACTER) {
            const auto pos1 = inManifold.GetWorldSpaceContactPointOn2(0);
            auto event1 = CollisionObject::Collision {
                .position = float3{pos1.GetX(), pos1.GetY(), pos1.GetZ()},
                .normal = normal,
                .object = node2
            };
            Application::callDeferred([this, signal, node1, event1]{
               node1->emit(signal, (void*)&event1);
            });
        }
        const auto pos2 = inManifold.GetWorldSpaceContactPointOn1(0);
        auto event2 = CollisionObject::Collision {
            .position = float3{pos2.GetX(), pos2.GetY(), pos2.GetZ()},
            .normal = normal,
            .object = node1
        };
        Application::callDeferred([this, signal, node2, event2]{
           node2->emit(signal, (void*)&event2);
        });
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(const JPH::ObjectLayer inObject1,
                                                  const JPH::ObjectLayer inObject2) const {
        return ObjectLayerPairFilterTable::ShouldCollide(inObject1, inObject2);
    }

    JoltPhysicsEngine::JoltPhysicsEngine(const LayerCollisionTable& layerCollisionTable):
        objectVsObjectLayerFilter{layerCollisionTable.layersCount} {
        // The layer vs layer collision table initialization
        for (const auto &layerCollide : layerCollisionTable.layersCollideWith) {
            for (const auto &layer : layerCollide.collideWith) {
                objectVsObjectLayerFilter.EnableCollision(layerCollide.layer, layer);
            }
        }

        // Initialize the Jolt Physics system
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);
        physicsSystem.Init(1024,
                           0,
                           2048,
                           104,
                           broadphaseLayerInterface,
                           objectVsBroadphaseLayerFilter,
                           objectVsObjectLayerFilter);
        physicsSystem.SetContactListener(&contactListener);
    }

    void JoltPhysicsEngine::update(const float deltaTime) {
        physicsSystem.Update(deltaTime, 1, tempAllocator.get(), jobSystem.get());
    }

    float3 JoltPhysicsEngine::getGravity() const {
        const auto gravity = physicsSystem.GetGravity();
        return float3{gravity.GetX(), gravity.GetY(), gravity.GetZ()};
    }

}