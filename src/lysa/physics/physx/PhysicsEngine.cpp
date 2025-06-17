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
    physx::PxFilterFlags myFilterShader(
        physx::PxFilterObjectAttributes attributes0,
        physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1,
        physx::PxFilterData filterData1,
        physx::PxPairFlags& pairFlags,
        const void* constantBlock,
        physx::PxU32 constantBlockSize) {
        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
        pairFlags |= physx::PxPairFlag::eMODIFY_CONTACTS;

        return physx::PxFilterFlag::eDEFAULT;
    }


    class MyContactModifyCallback : public physx::PxContactModifyCallback {
    public:
        void onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 count) override {
            for (int i = 0; i < count; ++i) {
                physx::PxContactModifyPair& pair = pairs[i];
                const physx::PxRigidDynamic* dynA =
                    pair.actor[0] ? pair.actor[0]->is<physx::PxRigidDynamic>() : nullptr;
                const physx::PxRigidDynamic* dynB =
                    pair.actor[1] ? pair.actor[1]->is<physx::PxRigidDynamic>() : nullptr;

                physx::PxVec3 vA = dynA ? dynA->getLinearVelocity() : physx::PxVec3(0.0f);
                physx::PxVec3 vB = dynB ? dynB->getLinearVelocity() : physx::PxVec3(0.0f);
                physx::PxVec3 relativeVelocity = vA - vB;

                for (physx::PxU32 contactIndex = 0; contactIndex < pair.contacts.size(); ++contactIndex) {
                    physx::PxVec3 normal = pair.contacts.getNormal(contactIndex);

                    float normalSpeed = relativeVelocity.dot(normal);
                    // if (physx::PxAbs(normalSpeed) < 5.0f) {
                        // pair.contacts.setRestitution(contactIndex, 0.0f);
                        // pair.contacts.setStaticFriction(contactIndex, 0.5f);
                        // pair.contacts.setDynamicFriction(contactIndex, 0.0f);
                    // }
                }
            }
        }
    };

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
        physx::PxDefaultCpuDispatcher* dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = dispatcher;
        sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
        // sceneDesc.filterShader = myFilterShader;
        // sceneDesc.contactModifyCallback = new MyContactModifyCallback();
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
