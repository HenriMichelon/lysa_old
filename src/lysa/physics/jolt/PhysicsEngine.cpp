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
import lysa.log;
import lysa.nodes.collision_object;
import lysa.nodes.node;
import lysa.physics.physics_material;

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
        emit(CollisionObject::on_collision_starts, inBody1, inBody2, inManifold, ioSettings);
    }

    void ContactListener::OnContactPersisted(
        const JPH::Body &inBody1,
        const JPH::Body &inBody2,
        const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings) {
        emit(CollisionObject::on_collision_persists, inBody1, inBody2, inManifold, ioSettings);
    }

    void ContactListener::emit(
        const Signal::signal &signal,
        const JPH::Body &body1,
        const JPH::Body &body2,
        const JPH::ContactManifold &inManifold,
        JPH::ContactSettings &ioSettings) const {
        const auto node1 = reinterpret_cast<CollisionObject*>(body1.GetUserData());
        const auto node2 = reinterpret_cast<CollisionObject*>(body2.GetUserData());
        assert([&]{ return node1 && node2; }, "physics body not associated with a node");

        const auto mat1 = reinterpret_cast<const PhysicsMaterial *>(
            body1.GetShape()->GetMaterial(inManifold.mSubShapeID1));
        const auto mat2 = reinterpret_cast<const PhysicsMaterial *>(
            body2.GetShape()->GetMaterial(inManifold.mSubShapeID2));
        if (mat1 && mat2) {
            ioSettings.mCombinedFriction = 0.5f * (
                body1.IsStatic() ? mat1->staticFriction : mat1->dynamicFriction +
                body2.IsStatic() ? mat2->staticFriction : mat2->dynamicFriction);
            ioSettings.mCombinedRestitution = std::max(mat1->restitution, mat2->restitution);
        }

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

    PhysicsMaterial::PhysicsMaterial(
        const float staticFriction,
        const float dynamicFriction,
        const float restitution):
        staticFriction(staticFriction),
        dynamicFriction(dynamicFriction),
        restitution(restitution) {
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
        defaultMaterial = JoltPhysicsEngine::createMaterial();
        JPH::PhysicsMaterial::sDefault = reinterpret_cast<JPH::PhysicsMaterial*>(defaultMaterial);
    }

    std::unique_ptr<PhysicsScene> JoltPhysicsEngine::createScene() {
        return std::make_unique<JoltPhysicsScene>(
            *tempAllocator,
            *jobSystem,
            contactListener,
            broadphaseLayerInterface,
            objectVsBroadphaseLayerFilter,
            objectVsObjectLayerFilter);
    }

    PhysicsMaterial* JoltPhysicsEngine::createMaterial(
        const float staticFriction,
        const float dynamicFriction,
        const float restitution) const {
        return new PhysicsMaterial(staticFriction, dynamicFriction, restitution);
    }

    PhysicsMaterial* JoltPhysicsEngine::duplicateMaterial(const PhysicsMaterial* orig) const {
        return new PhysicsMaterial(orig->staticFriction, orig->dynamicFriction, orig->restitution);
    }

    JoltPhysicsScene::JoltPhysicsScene(
        JPH::TempAllocatorImpl& tempAllocator,
        JPH::JobSystemThreadPool& jobSystem,
        ContactListener& contactListener,
        const BPLayerInterfaceImpl& broadphaseLayerInterface,
        const ObjectVsBroadPhaseLayerFilterImpl& objectVsBroadphaseLayerFilter,
        const ObjectLayerPairFilterImpl& objectVsObjectLayerFilter) :
        tempAllocator {tempAllocator},
        jobSystem {jobSystem} {
        physicsSystem.Init(1024,
                           0,
                           2048,
                           104,
                           broadphaseLayerInterface,
                           objectVsBroadphaseLayerFilter,
                           objectVsObjectLayerFilter);
        physicsSystem.SetContactListener(&contactListener);
    }

    void JoltPhysicsScene::update(const float deltaTime) {
        physicsSystem.Update(deltaTime, 1, &tempAllocator, &jobSystem);
    }

    float3 JoltPhysicsScene::getGravity() const {
        const auto gravity = physicsSystem.GetGravity();
        return float3{gravity.GetX(), gravity.GetY(), gravity.GetZ()};
    }

}