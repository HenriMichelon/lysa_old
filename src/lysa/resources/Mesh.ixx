/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.mesh;
#include <cassert>

import std;
import vireo;
import lysa.aabb;
import lysa.global;
import lysa.resources.material;
import lysa.resources.resource;

export namespace lysa {

    /**
     * %A Mesh vertex
     */
    struct Vertex {
        //! local position
        float3 position;
        //! surface normal
        float3 normal;
        //! UV coordinates in the surface
        float2 uv;
        //! UV-based tangents
        float3 tangent;

        inline bool operator==(const Vertex &other) const {
            return all(position == other.position) &&
                    all(normal == other.normal) &&
                    all(uv == other.uv) &&
                    all(tangent == other.tangent);
        }
    };

    /**
     * %A Mesh surface, with counterclockwise triangles
     */
    struct MeshSurface {
        //! Index of the first vertex of the surface
        uint32_t                  firstVertexIndex{0};
        //! Number of vertices
        uint32_t                  indexCount{0};
        //! Material
        std::shared_ptr<Material> material{};

        MeshSurface(uint32_t firstIndex, uint32_t count);

        inline bool operator==(const MeshSurface &other) const {
            return firstVertexIndex == other.firstVertexIndex && indexCount == other.indexCount && material == other.material;
        }

        friend inline bool operator==(const std::shared_ptr<MeshSurface>& a, const std::shared_ptr<MeshSurface>& b) {
            return *a == *b;
        }
    };

    /**
     * %A mesh composed by multiple Surface and an indexes collection of Vertex
     */
    class Mesh : public Resource {
    public:
        inline static const std::vector<vireo::VertexAttributeDesc> vertexAttributes{
            {"POSITION", vireo::AttributeFormat::R32G32B32_FLOAT, offsetof(Vertex, position) },
            {"NORMAL",   vireo::AttributeFormat::R32G32B32_FLOAT, offsetof(Vertex, normal)},
            {"UV",       vireo::AttributeFormat::R32G32_FLOAT,   offsetof(Vertex, uv)},
            {"TANGENT",  vireo::AttributeFormat::R32G32B32_FLOAT, offsetof(Vertex, tangent)},
        };

        /**
         * Creates an empty Mesh
         * @param name resource name
         */
        Mesh(const std::wstring &name = L"Mesh");

        /**
         * Creates a Mesh from vertices
         * @param vertices Vertices
         * @param indices Indexes of vertices
         * @param surfaces Surfaces
         * @param name Node name
         */
        Mesh(const std::vector<Vertex>& vertices,
             const std::vector<uint32_t>& indices,
             const std::vector<std::shared_ptr<MeshSurface>>&surfaces,
             const std::wstring& name = L"Mesh");

         /**
         * Returns the material for a given surface
         * @param surfaceIndex Zero-based index of the surface
         */
        const std::shared_ptr<Material>& getSurfaceMaterial(const uint32_t surfaceIndex) const {
            assert(surfaceIndex < surfaces.size());
            return surfaces[surfaceIndex]->material;
        }

        /**
         * Changes the material of a given surface
         * @param surfaceIndex Zero-based index of the Surface
         * @param material New material for the Surface
         */
        void setSurfaceMaterial(uint32_t surfaceIndex, const std::shared_ptr<Material>& material);

        /**
         * Returns all the Surfaces
         */
        std::vector<std::shared_ptr<MeshSurface>>& getSurfaces() { return surfaces; }

        /**
         * Returns all the vertices
         */
        std::vector<Vertex>& getVertices() { return vertices; }

        /**
         * Return all the vertices indexes
         */
        std::vector<uint32_t>& getIndices() { return indices; }

        /**
         * Returns all the vertices
         */
        const std::vector<Vertex>& getVertices() const { return vertices; }

        /**
         * Return all the vertices indexes
         */
        const std::vector<uint32_t>& getIndices() const { return indices; }

        /**
         * Returns the local space axis aligned bounding box
         */
        const AABB& getAABB() const { return localAABB; }

        bool operator==(const Mesh &other) const;

        friend inline bool operator==(const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
            return (a == nullptr) ? false : *a == *b;
        }

        friend inline bool operator<(const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
            return *a < *b;
        }

    protected:
        AABB                  localAABB;
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;

        std::vector<std::shared_ptr<MeshSurface>>     surfaces{};
        std::unordered_set<std::shared_ptr<Material>> materials{};

        std::shared_ptr<vireo::Buffer> vertexBuffer;
        std::shared_ptr<vireo::Buffer> indexBuffer;

        void buildAABB();

    private:
        void upload();

        void bind(const vireo::CommandList& commandList) const;

        std::unordered_set<std::shared_ptr<Material>>& getMaterials() { return materials; }
    };
}

