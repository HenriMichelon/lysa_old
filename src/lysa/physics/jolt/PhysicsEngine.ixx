/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
export module lysa.physics.jolt.engine;

import std;
import lysa.math;
import lysa.signal;
import lysa.types;
import lysa.physics.configuration;
import lysa.physics.engine;
import lysa.physics.physics_material;

export namespace lysa {

    // Class that determines if two nodes can collide
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilterTable {
    public:
        ObjectLayerPairFilterImpl(const uint32 inNumObjectLayers): ObjectLayerPairFilterTable(inNumObjectLayers) {}
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
    };

    // This defines a mapping between objects and broadphase layers.
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        uint32_t GetNumBroadPhaseLayers() const override { return 1;}
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { return static_cast<JPH::BroadPhaseLayer>(0); }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override { return "?";}
#endif

    };

    // Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer layers, JPH::BroadPhaseLayer masks) const override { return true; }
    };

    class ContactListener : public JPH::ContactListener {
    public:
        JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1,
                                              const JPH::Body &inBody2,
                                              JPH::RVec3Arg inBaseOffset,
                                              const JPH::CollideShapeResult &inCollisionResult) override;
        void OnContactAdded(const JPH::Body &inBody1,
                            const JPH::Body &inBody2,
                            const JPH::ContactManifold &inManifold,
                            JPH::ContactSettings &ioSettings) override;
        void OnContactPersisted(const JPH::Body &inBody1, 
                                const JPH::Body &inBody2, 
                                const JPH::ContactManifold &inManifold, 
                                JPH::ContactSettings &ioSettings) override;

    private:
        void emit(const Signal::signal &signal,
                  const JPH::Body &body1, 
                  const JPH::Body &body2, 
                  const JPH::ContactManifold &inManifold,
                  JPH::ContactSettings &ioSettings) const;
    };

    class JoltPhysicsScene : public PhysicsScene {
    public:
        JoltPhysicsScene(
            JPH::TempAllocatorImpl& tempAllocator,
            JPH::JobSystemThreadPool& jobSystem,
            ContactListener& contactListener,
            const BPLayerInterfaceImpl& broadphaseLayerInterface,
            const ObjectVsBroadPhaseLayerFilterImpl& objectVsBroadphaseLayerFilter,
            const ObjectLayerPairFilterImpl& objectVsObjectLayerFilter
        );

        void update(float deltaTime) override;

        float3 getGravity() const override;

        auto& getBodyInterface() { return physicsSystem.GetBodyInterface(); }

        auto& getPhysicsSystem() { return physicsSystem; }

        auto& getTempAllocator() { return tempAllocator; }

    private:
        JPH::PhysicsSystem physicsSystem;
        JPH::TempAllocatorImpl& tempAllocator;
        JPH::JobSystemThreadPool& jobSystem;
    };

    class JoltPhysicsEngine : public PhysicsEngine {
    public:
        JoltPhysicsEngine(const LayerCollisionTable& layerCollisionTable);

        std::unique_ptr<PhysicsScene> createScene() override;

        PhysicsMaterial* createMaterial(
            float staticFriction = 0.5f,
            float dynamicFriction = 0.5f,
            float restitution = 0.0f) override;

        auto& getObjectLayerPairFilter() { return objectVsObjectLayerFilter; }

    private:
        ContactListener contactListener;
        BPLayerInterfaceImpl broadphaseLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter;
        ObjectLayerPairFilterImpl objectVsObjectLayerFilter;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
        PhysicsMaterial* defaultMaterial;
    };

}