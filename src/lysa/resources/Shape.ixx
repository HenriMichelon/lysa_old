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

    protected:
        PhysicsMaterial* material;

#ifdef PHYSIC_ENGINE_JOLT
    public:
        auto getShapeSettings() const { return shapeSettings; }
    protected:
        JPH::ShapeSettings* shapeSettings{nullptr};
#endif
#ifdef PHYSIC_ENGINE_PHYSX
    public:
        auto getShape() const { return shape; }
    protected:
        physx::PxShape* shape{nullptr};
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
        explicit BoxShape(
            const float3& extends,
            PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"BoxShape");

        std::shared_ptr<Resource> duplicate() const override;

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

    private:
        SphereShape(const std::wstring &resName, PhysicsMaterial* material) : Shape(material, resName) {}
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

    private:
        AABBShape(
            PhysicsMaterial* material,
            const std::wstring &resName) :
            Shape{material, resName} {}
    };

}
