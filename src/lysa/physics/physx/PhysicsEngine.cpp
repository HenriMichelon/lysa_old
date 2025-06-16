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
import lysa.log;
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
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 2.0f);
    }

    PhysXPhysicsScene::~PhysXPhysicsScene() {
        scene->release();
    }

    void PhysXPhysicsScene::update(const float deltaTime) {
        scene->simulate(deltaTime);
        scene->fetchResults(true);
    }

    void PhysXPhysicsScene::debug(DebugRenderer& debugRenderer) {
        const physx::PxRenderBuffer& rb = scene->getRenderBuffer();
        for(int i=0; i < rb.getNbLines(); i++) {
            const auto& line = rb.getLines()[i];
            debugRenderer.drawLine(
                float3{line.pos0.x, line.pos0.y, line.pos0.z},
                float3{line.pos1.x, line.pos1.y, line.pos1.z},
                float4{line.color0, line.color0, line.color0, 1.0f});
        }
        for(int i=0; i < rb.getNbTriangles(); i++) {
            const auto& triangle = rb.getTriangles()[i];
            debugRenderer.drawTriangle(
                float3{triangle.pos0.x, triangle.pos0.y, triangle.pos0.z},
                float3{triangle.pos1.x, triangle.pos1.y, triangle.pos1.z},
                float3{triangle.pos2.x, triangle.pos2.y, triangle.pos2.z},
                float4{triangle.color0, triangle.color0, triangle.color0, 1.0f});
        }
    }

    float3 PhysXPhysicsScene::getGravity() const {
        const auto gravity = scene->getGravity();
        return float3{gravity.x, gravity.y, gravity.z};
    }

}