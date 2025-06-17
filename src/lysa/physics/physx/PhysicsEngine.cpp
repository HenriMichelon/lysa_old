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

    bool PhysXPhysicsEngine::collisionMatrix[MAX_COLLISIONS_LAYERS][MAX_COLLISIONS_LAYERS]{};

    physx::PxFilterFlags myFilterShader(
        const physx::PxFilterObjectAttributes,
        const physx::PxFilterData filterData0,
        const physx::PxFilterObjectAttributes,
        const physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags,
        const void*, physx::PxU32) {
        if (!PhysXPhysicsEngine::collisionMatrix[filterData0.word0][filterData0.word0]) {
            return physx::PxFilterFlag::eKILL;
        }
        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
        // pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
        // pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
        return physx::PxFilterFlag::eDEFAULT;
    }


    PhysXPhysicsEngine::PhysXPhysicsEngine(const LayerCollisionTable& layerCollisionTable) {
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
        if (!foundation) {
            throw Exception("Failed to create PhysX foundation");
        }
        physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale());
        if (!foundation) {
            throw Exception("Failed to create PhysX physics");
        }

        // Build collision matrix
        for (uint32_t i = 0; i < MAX_COLLISIONS_LAYERS; ++i) {
            for (uint32_t j = 0; j < MAX_COLLISIONS_LAYERS; ++j) {
                collisionMatrix[i][j] = false;
            }
        }
        for (const auto& entry : layerCollisionTable.layersCollideWith) {
            const uint32_t from = entry.layer;
            for (const uint32_t to : entry.collideWith) {
                collisionMatrix[from][to] = true;
            }
        }
    }

    PhysXPhysicsEngine::~PhysXPhysicsEngine() {
        physics->release();
        foundation->release();
    }

    std::unique_ptr<PhysicsScene> PhysXPhysicsEngine::createScene(const DebugConfig& debugConfig) {
        return std::make_unique<PhysXPhysicsScene>(physics, debugConfig);
    }

    PhysicsMaterial* PhysXPhysicsEngine::createMaterial(
        const float friction,
        const float restitution) const {
        const auto material = physics->createMaterial(friction, friction, restitution);
        material->setFrictionCombineMode(physx::PxCombineMode::eAVERAGE);
        material->setRestitutionCombineMode(physx::PxCombineMode::eMAX);
        return material;
    }

    void PhysXPhysicsEngine::setRestitutionCombineMode(PhysicsMaterial* physicsMaterial,
                                                       const CombineMode combineMode) const {
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

    PhysXPhysicsScene::PhysXPhysicsScene(
        physx::PxPhysics* physics,
        const DebugConfig& debugConfig) {
        physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());
        sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        sceneDesc.filterShader = myFilterShader;
        scene = physics->createScene(sceneDesc);
        controllerManager = PxCreateControllerManager(*scene);
        if (debugConfig.enabled) {
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eWORLD_AXES,
                debugConfig.drawCoordinateSystem ? debugConfig.coordinateSystemScale : 0.0f);
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES,
                debugConfig.drawCenterOfMass);
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES,
                debugConfig.drawShape);
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_AABBS,
                debugConfig.drawBoundingBox);
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_LIN_VELOCITY,
                debugConfig.drawVelocity);
            scene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_ANG_VELOCITY,
               debugConfig.drawVelocity);
        }
    }

    PhysXPhysicsScene::~PhysXPhysicsScene() {
        controllerManager->release();
        scene->release();
    }

    void PhysXPhysicsScene::update(const float deltaTime) {
        scene->simulate(deltaTime);
        scene->fetchResults(true);
    }

    void PhysXPhysicsScene::debug(DebugRenderer& debugRenderer) {
        const physx::PxRenderBuffer& rb = scene->getRenderBuffer();
        for (int i = 0; i < rb.getNbLines(); i++) {
            const auto& line = rb.getLines()[i];
            debugRenderer.drawLine(
                float3{line.pos0.x, line.pos0.y, line.pos0.z},
                float3{line.pos1.x, line.pos1.y, line.pos1.z},
                colorU32ToFloat4(line.color0));
        }
        for (int i = 0; i < rb.getNbTriangles(); i++) {
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
        return float4{r, g, b, a};
    }

    float3 PhysXPhysicsScene::getGravity() const {
        const auto gravity = scene->getGravity();
        return float3{gravity.x, gravity.y, gravity.z};
    }
}
