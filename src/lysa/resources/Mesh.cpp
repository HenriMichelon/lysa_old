/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cstddef>
module lysa.resources.mesh;

import lysa.application;
import lysa.global;
import lysa.log;
import lysa.window;

namespace lysa {

    const std::vector<vireo::VertexAttributeDesc> VertexData::vertexAttributes {
        {"POSITION", vireo::AttributeFormat::R32G32B32A32_FLOAT, offsetof(VertexData, position)},
        {"NORMAL", vireo::AttributeFormat::R32G32B32A32_FLOAT, offsetof(VertexData, normal)},
        {"TANGENT", vireo::AttributeFormat::R32G32B32A32_FLOAT, offsetof(VertexData, tangent)},
    };

    MeshSurface::MeshSurface(const uint32 firstIndex, const uint32 count):
        firstIndex{firstIndex},
        indexCount{count},
        material{nullptr} {
    }

    Mesh::Mesh(const std::wstring &name):
        Resource{name} {
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<uint32>& indices,
               const std::vector<std::shared_ptr<MeshSurface>> &surfaces,
               const std::wstring& name):
        Resource{name},
        vertices{vertices},
        indices{indices},
        surfaces{surfaces} {
        buildAABB();
    }

    void Mesh::setSurfaceMaterial(const uint32 surfaceIndex, const std::shared_ptr<Material>& material) {
        assert([&]{return surfaceIndex < surfaces.size();}, "Invalid surface index");
        surfaces[surfaceIndex]->material = material;
        materials.insert(surfaces[surfaceIndex]->material);
        upload();
    }

    void Mesh::upload() {
        auto& resources = Application::getResources();
        if (!isUploaded()) {
            verticesMemoryBlock = resources.getVertexArray().alloc(vertices.size());
            indicesMemoryBlock = resources.getIndexArray().alloc(indices.size());
            surfacesMemoryBlock = resources.getMeshSurfaceArray().alloc(surfaces.size());
        }

        // Uploading all vertices
        auto vertexData = std::vector<VertexData>(vertices.size());
        for (int i = 0; i < vertices.size(); i++) {
            const auto& v = vertices[i];
            vertexData[i].position = float4(v.position.x, v.position.y, v.position.z, v.uv.x);
            vertexData[i].normal = float4(v.normal.x, v.normal.y, v.normal.z, v.uv.y);
            vertexData[i].tangent = v.tangent;
        }
        resources.getVertexArray().write(verticesMemoryBlock, vertexData.data());

        // Uploading all indices
        resources.getIndexArray().write(indicesMemoryBlock, indices.data());

        // Uploading all surfaces & materials
        auto surfaceData = std::vector<MeshSurfaceData>(surfaces.size());
        for (int i = 0; i < surfaces.size(); i++) {
            const auto& surface = surfaces[i];
            const auto& material = surface->material;
            if (!material->isUploaded()) {
               material->upload();
            }
            surfaceData[i].indexCount = surface->indexCount;
            surfaceData[i].indicesIndex = indicesMemoryBlock.instanceIndex + surface->firstIndex;
            surfaceData[i].verticesIndex = verticesMemoryBlock.instanceIndex;
        }
        resources.getMeshSurfaceArray().write(surfacesMemoryBlock, surfaceData.data());
        resources.setUpdated();
    }

    bool Mesh::operator==(const Mesh &other) const {
        return vertices == other.vertices &&
               indices == other.indices &&
               surfaces == other.surfaces &&
               materials == other.materials;
    }

    void Mesh::buildAABB() {
        auto min = float3{std::numeric_limits<float>::max()};
        auto max = float3{std::numeric_limits<float>::lowest()};
        for (const auto index : indices) {
            const auto& position = vertices[index].position;
            //Get the smallest vertex
            min.x = std::min(min.x, position.x);    // Find smallest x value in model
            min.y = std::min(min.y, position.y);    // Find smallest y value in model
            min.z = std::min(min.z, position.z);    // Find smallest z value in model
            //Get the largest vertex
            max.x = std::max(max.x, position.x);    // Find largest x value in model
            max.y = std::max(max.y, position.y);    // Find largest y value in model
            max.z = std::max(max.z, position.z);    // Find largest z value in model
        }
        localAABB = {min, max};
    }

}
