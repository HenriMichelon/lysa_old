/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.physics.physx.engine;

import lysa.application;
import lysa.global;
import lysa.nodes.collision_object;
import lysa.nodes.node;
import lysa.physics.physics_material;

namespace lysa {

    PhysXPhysicsEngine::PhysXPhysicsEngine() {
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
        if (!foundation) {
            throw Exception("Failed to create PhysX foundation");
        }
        physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale());
        if (!foundation) {
            throw Exception("Failed to create PhysX physics");
        }
    }

    PhysXPhysicsEngine::~PhysXPhysicsEngine() {
        physics->release();
        foundation->release();
    }

    std::unique_ptr<PhysicsScene> PhysXPhysicsEngine::createScene() {
        return std::make_unique<PhysXPhysicsScene>(physics);
    }

    PhysicsMaterial* PhysXPhysicsEngine::createMaterial(
       const float staticFriction,
       const float dynamicFriction,
       const float restitution) const {
        return physics->createMaterial(staticFriction, dynamicFriction, restitution);
    }

    PhysicsMaterial* PhysXPhysicsEngine::duplicateMaterial(const PhysicsMaterial* orig) const {
        return physics->createMaterial(
            orig->getStaticFriction(),
            orig->getDynamicFriction(),
            orig->getRestitution());
    }

    PhysXPhysicsScene::PhysXPhysicsScene(physx::PxPhysics* physics) {
        physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());
        sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = dispatcher;
        sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
        scene = physics->createScene(sceneDesc);
    }

    PhysXPhysicsScene::~PhysXPhysicsScene() {
        scene->release();
    }

    void PhysXPhysicsScene::update(const float deltaTime) {
        scene->simulate(deltaTime);
        scene->fetchResults(true);
    }

    float3 PhysXPhysicsScene::getGravity() const {
        const auto gravity = scene->getGravity();
        return float3{gravity.x, gravity.y, gravity.z};
    }

}