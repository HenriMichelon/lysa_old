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
import lysa.configuration;
import lysa.math;
import lysa.signal;
import lysa.types;
import lysa.physics.configuration;
import lysa.physics.engine;
import lysa.physics.physics_material;
import lysa.renderers.debug;

export namespace lysa {

    /**
     * Object layer collision filter for the Jolt backend.
     * Decides if two object layers are allowed to collide (narrow phase hint).
     */
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilterTable {
    public:
        ObjectLayerPairFilterImpl(const uint32 inNumObjectLayers): ObjectLayerPairFilterTable(inNumObjectLayers) {}
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
    };

    /**
     * Broad‑phase layer interface mapping all objects to a single broad‑phase layer.
     */
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        uint32_t GetNumBroadPhaseLayers() const override { return 1;}
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { return static_cast<JPH::BroadPhaseLayer>(0); }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override { return "?";}
#endif

    };

    /**
     * Filter that determines if an object layer can collide with a broad‑phase layer.
     * Here everything collides with the single broad‑phase layer.
     */
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer layers, JPH::BroadPhaseLayer masks) const override { return true; }
    };

    /**
     * Jolt contact listener used to forward physics events to the engine's Signal system.
     */
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

    /**
     * Jolt implementation of PhysicsScene.
     * Owns the Jolt PhysicsSystem and provides update/debug helpers.
     */
    class JoltPhysicsScene : public PhysicsScene {
    public:
        JoltPhysicsScene(
            const DebugConfig& debugConfig,
            JPH::TempAllocatorImpl& tempAllocator,
            JPH::JobSystemThreadPool& jobSystem,
            ContactListener& contactListener,
            const BPLayerInterfaceImpl& broadphaseLayerInterface,
            const ObjectVsBroadPhaseLayerFilterImpl& objectVsBroadphaseLayerFilter,
            const ObjectLayerPairFilterImpl& objectVsObjectLayerFilter
        );

        void update(float deltaTime) override;

        void debug(DebugRenderer& debugRenderer) override;

        float3 getGravity() const override;

        /** Returns the body interface for creating/removing bodies. */
        auto& getBodyInterface() { return physicsSystem.GetBodyInterface(); }

        /** Returns the underlying Jolt physics system. */
        auto& getPhysicsSystem() { return physicsSystem; }

        /** Returns the temporary allocator used by the scene. */
        auto& getTempAllocator() const { return tempAllocator; }

    private:
        const DebugConfig& debugConfig;
        JPH::PhysicsSystem physicsSystem;
        JPH::TempAllocatorImpl& tempAllocator;
        JPH::JobSystemThreadPool& jobSystem;
        // Debug view config
        JPH::BodyManager::DrawSettings bodyDrawSettings{};
    };

    /**
     * Jolt implementation of PhysicsEngine.
     * Creates scenes/materials and wires Jolt‑specific collision filtering.
     */
    class JoltPhysicsEngine : public PhysicsEngine {
    public:
        JoltPhysicsEngine(const LayerCollisionTable& layerCollisionTable);

        std::unique_ptr<PhysicsScene> createScene(const DebugConfig& debugConfig) override;

        PhysicsMaterial* createMaterial(
            float friction = 0.5f,
            float restitution = 0.0f) const override;

        PhysicsMaterial* duplicateMaterial(const PhysicsMaterial* orig) const override;

        void setRestitutionCombineMode(PhysicsMaterial* physicsMaterial, CombineMode combineMode) const override;

        /** Returns the pair filter used to decide if two object layers collide. */
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