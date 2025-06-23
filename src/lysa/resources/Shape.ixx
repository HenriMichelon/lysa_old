/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#endif
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.resources.shape;

import std;
import lysa.math;
import lysa.nodes.node;
import lysa.physics.physics_material;
import lysa.resources.resource;
#ifdef PHYSIC_ENGINE_PHYSX
import lysa.application;
import lysa.physics.physx.engine;
#endif

export namespace lysa {

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    public:
        Shape(const PhysicsMaterial* material, const std::wstring &resName);

        auto& getMaterial() const { return *material; }

    protected:
        PhysicsMaterial* material;

#ifdef PHYSIC_ENGINE_JOLT
    public:
        virtual JPH::ShapeSettings* getShapeSettings() { return shapeSettings; }
    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};
#endif
#ifdef PHYSIC_ENGINE_PHYSX
    public:
        virtual std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const { return nullptr; }
    protected:
        // std::unique_ptr<physx::PxGeometry> geometry;
        static auto getPhysx() {
            return dynamic_cast<PhysXPhysicsEngine&>(Application::getPhysicsEngine()).getPhysics();
        }
#endif
    };

    /**
     * Box-shaped collision Shape
     */
    class BoxShape : public Shape {
    public:
        /**
         * Creates a BoxShape with the given extents
         */
        BoxShape(
            const float3& extends,
            PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"BoxShape");

        std::shared_ptr<Resource> duplicate() const override;

#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        const float3 extends;
    };

    /**
     * Sphere-shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        /**
         * Creates a SphereShape with the given radius
         */
        SphereShape(
            float radius,
            const PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"SphereShape");

#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        const float radius;
        // SphereShape(const std::wstring &resName, PhysicsMaterial* material) : Shape(material, resName), radius {0}{}
    };

    /**
     * AABB-based collision Shape
     */
    class AABBShape : public Shape {
    public:
        /**
         * Creates an AABBShape for a given node
         */
        AABBShape(
            const std::shared_ptr<Node> &node,
            const PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"AABBShape");

        /**
         * Creates an AABBShape for a given node
         */
        AABBShape(
            const Node &node,
            const PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"AABBShape");

#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        float3 extends;
        AABBShape(
            PhysicsMaterial* material,
            const std::wstring &resName) :
            Shape{material, resName} {}
    };

}
