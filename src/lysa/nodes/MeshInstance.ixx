/*
* Copyright (c) 2025-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.mesh_instance;

import lysa.global;
import lysa.aabb;
import lysa.nodes.node;
import lysa.resources.material;
import lysa.resources.mesh;

export namespace lysa {

    struct MeshInstanceData {
        float4x4 transform;
        float3   aabbMin;
        float3   aabbMax;
        uint     visible;
    };

    /**
     * Node that holds a Mesh.
     */
    class MeshInstance : public Node {
    public:
        /**
         * Creates a MeshInstance with the given Mesh
         */
        MeshInstance(const std::shared_ptr<Mesh>& mesh, const std::wstring &name = TypeNames[MESH_INSTANCE]);

        /**
         * Returns the associated Mesh
         */
        const auto& getMesh() const { return mesh; }

        /**
         * Set to `true` to have the Mesh outlined starting to the next frame
         */
        // void setOutlined(const bool o) { outlined = o; }

        /**
         * Returns `true` if the Mesh is outlined during the next frame
         */
        // auto isOutlined() const { return outlined; }

        /**
         * Sets the outline material. The material **must** belong to the OutlineMaterials collection.
         */
        // void setOutlineMaterial(const std::shared_ptr<ShaderMaterial> &material) { outlineMaterial = material; }

        /**
         * Returns the current outlining material
         */
        // auto& getOutlineMaterial() { return outlineMaterial; }

        /**
         * Returns the world space axis aligned bounding box
         */
        const auto& getAABB() const { return worldAABB; }

        MeshInstanceData getModelData() const;

        friend inline bool operator<(const MeshInstance& a, const MeshInstance& b) {
            return a.mesh < b.mesh;
        }

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    private:
        AABB worldAABB;
        std::shared_ptr<Mesh> mesh;

        // bool                            outlined{false};
        // std::shared_ptr<ShaderMaterial> outlineMaterial;

        void updateGlobalTransform() override;
    };

}
