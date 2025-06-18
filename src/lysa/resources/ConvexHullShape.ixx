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
export module lysa.resources.convex_hull_shape;

import std;
import lysa.math;
import lysa.nodes.node;
import lysa.nodes.mesh_instance;
import lysa.physics.physics_material;
import lysa.resources.shape;

export namespace lysa {

    /**
     * %A convex hull collision shape
     */
    class ConvexHullShape : public Shape {
    public:
        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of the first MeshInstance found in the `node` tree.
         * Uses the local transform of the node when creating the shape.
         */
        ConvexHullShape(
            const std::shared_ptr<Node> &node,
            const PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"ConvexHullShape");

        std::shared_ptr<Resource> duplicate()  const override;

#ifdef PHYSIC_ENGINE_JOLT
        JPH::ShapeSettings* getShapeSettings() override;
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        std::unique_ptr<physx::PxGeometry> getGeometry(const float3& scale) const override;
#endif
    private:
        std::shared_ptr<MeshInstance> meshInstance;
    };

}
