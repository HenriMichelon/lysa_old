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
export module lysa.resources.mesh_shape;

import lysa.nodes.node;
import lysa.nodes.mesh_instance;
import lysa.physics.physics_material;
import lysa.resources.shape;

export namespace lysa {

    /**
     * %A mesh shape, consisting of triangles. *Must* only be used with a StaticBody (like a terrain for example)
     */
    class MeshShape : public Shape {
    public:
        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        MeshShape(
            const std::shared_ptr<Node> &node,
            const PhysicsMaterial* material = nullptr,
            const std::wstring &resName = L"MeshShape");

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
