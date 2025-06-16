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

    void PhysXPhysicsEngine::setRestitutionCombineMode(PhysicsMaterial* physicsMaterial, CombineMode combineMode) const {
        physx::PxCombineMode::Enum pxCombineMode;
        switch (combineMode) {
        case CombineMode::AVERAGE:
            pxCombineMode = physx::PxCombineMode::eAVERAGE;
            break;
        case CombineMode::MIN:
            pxCombineMode = physx::PxCombineMode::eMIN;
            break;
        case CombineMode::MAX:
            pxCombineMode = physx::PxCombineMode::eMAX;
            break;
        case CombineMode::MULTIPLY:
            pxCombineMode = physx::PxCombineMode::eMULTIPLY;
            break;
        default:
            return;
        }
        physicsMaterial->setRestitutionCombineMode(pxCombineMode);
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
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eWORLD_AXES, 2.0f);
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 1.0f);
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
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
                colorU32ToFloat4(line.color0));
        }
        for(int i=0; i < rb.getNbTriangles(); i++) {
            const auto& triangle = rb.getTriangles()[i];
            debugRenderer.drawTriangle(
                float3{triangle.pos0.x, triangle.pos0.y, triangle.pos0.z},
                float3{triangle.pos1.x, triangle.pos1.y, triangle.pos1.z},
                float3{triangle.pos2.x, triangle.pos2.y, triangle.pos2.z},
                colorU32ToFloat4(triangle.color0));
        }
    }

    float4 PhysXPhysicsScene::colorU32ToFloat4(physx::PxU32 color) {
        const float a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
        const float r = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
        const float g = static_cast<float>((color >> 8) & 0xFF) / 255.0f;
        const float b = static_cast<float>((color >> 0) & 0xFF) / 255.0f;
        return float4{ r, g, b, a };
    }

    float3 PhysXPhysicsScene::getGravity() const {
        const auto gravity = scene->getGravity();
        return float3{gravity.x, gravity.y, gravity.z};
    }

}