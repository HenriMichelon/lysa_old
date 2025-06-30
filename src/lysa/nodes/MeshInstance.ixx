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
        uint     castShadows;
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
         * Returns the world space axis aligned bounding box
         */
        const auto& getAABB() const { return worldAABB; }

        MeshInstanceData getModelData() const;

        friend inline bool operator<(const MeshInstance& a, const MeshInstance& b) {
            return a.mesh < b.mesh;
        }

        const auto& getSurfaceOverrideMaterials() const { return overrideMaterials; }

        void setSurfaceOverrideMaterial(uint32 surfaceIndex, const std::shared_ptr<Material>& material);

        std::shared_ptr<Material> getSurfaceOverrideMaterial(uint32 surfaceIndex);

        /**
         * Changes the material of a given surface
         * @param surfaceIndex Zero-based index of the Surface
         * @param material New material for the Surface
         */
        void setSurfaceMaterial(uint32 surfaceIndex, const std::shared_ptr<Material>& material);

        /**
        * Returns the material for a given surface
        * @param surfaceIndex Zero-based index of the surface
        */
        const std::shared_ptr<Material>& getSurfaceMaterial(uint32 surfaceIndex) const;

        void setCastShadows(bool castShadows);

        auto getCastShadows() const { return castShadows; }

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

        void setProperty(const std::string &property, const std::string &value) override;

    private:
        bool castShadows{true};
        AABB worldAABB;
        std::shared_ptr<Mesh> mesh;
        std::unordered_map<uint32, std::shared_ptr<Material>> overrideMaterials;

        void updateGlobalTransform() override;
    };

}
