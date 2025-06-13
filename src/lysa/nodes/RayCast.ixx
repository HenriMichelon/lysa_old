/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#endif
export module lysa.nodes.ray_cast;

import lysa.math;
import lysa.nodes.collision_object;
import lysa.nodes.node;

export namespace lysa {

    /**
     * %A ray in 3D space, used to find the first CollisionObject it intersects.
     */
    class RayCast : public Node, public JPH::BodyFilter {
    public:
        /**
         * Creates a RayCast
         * @param target The ray's destination point, relative to the RayCast's position
         * @param layer The ray's collision layer
         * @param name The node's name
         */
        RayCast(const float3& target, collision_layer layer, const std::wstring& name = TypeNames[RAYCAST]);

        /**
         * Creates a RayCast with a [0.0, 0.0, 0.0] target
         * @param name The node's name
         */
        RayCast(const std::wstring &name = TypeNames[RAYCAST]);

        /**
         * Returns whether any object is intersecting with the ray's vector (considering the vector length).
         */
        auto isColliding() const { return collider != nullptr; }

        /**
         * Returns the first object that the ray intersects, or `nullptr` if no object is intersecting the ray
         */
        auto getCollider() const { return collider; }

        /**
         * Returns the collision point at which the ray intersects the closest object, in the global coordinate system
         */
        auto getCollisionPoint() const { return hitPoint; }

        /**
         * If `true`, collisions will be ignored for this RayCast's immediate parent.
         */
        auto setExcludeParent(const bool exclude) { excludeParent = exclude; }

        /**
         * Updates the collision information for the ray immediately, 
         * without waiting for the next physics update
         */
        auto forceUpdate() { physicsProcess(0.0f); }

        /**
         * Sets the ray's destination point, relative to the RayCast's position.
         */
        auto setTarget(const float3& target) { this->target = target; }

        /**
         * Returns the ray target
         */
        const auto& getTarget() const { return this->target; }

        void setCollisionLayer(collision_layer layer);

    protected:
        float3 target{};
        float3 hitPoint{};
        collision_layer collisionLayer{};
        bool excludeParent{true};

        void physicsProcess(float delta) override;

#ifdef PHYSIC_ENGINE_JOLT
        std::shared_ptr<CollisionObject> collider{nullptr};
        JPH::BroadPhaseLayerFilter broadPhaseLayerFilter{};
        std::unique_ptr<JPH::ObjectLayerFilter> objectLayerFilter;
        bool ShouldCollideLocked(const JPH::Body &inBody) const override;
#endif
    };

}
