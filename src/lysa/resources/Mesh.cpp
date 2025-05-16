/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.mesh;

import lysa.global;
import lysa.window;

namespace lysa {

    MeshSurface::MeshSurface(const uint32 firstIndex, const uint32 count):
        firstIndex{firstIndex},
        indexCount{count},
        material{nullptr} {
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

    Mesh::Mesh(const std::vector<Vertex>& vertices,
            const std::vector<uint32>& indices,
            const std::vector<std::shared_ptr<MeshSurface>>&surfaces,
            const uint32 firstIndex,
            const int32 vertexOffset,
            const std::shared_ptr<vireo::Buffer>& vertexBuffer,
            const std::shared_ptr<vireo::Buffer>& indexBuffer,
            const std::wstring &name):
        Resource{name},
        vertices{vertices},
        indices{indices},
        surfaces{surfaces},
        firstIndex{firstIndex},
        vertexOffset{vertexOffset},
        vertexBuffer{vertexBuffer},
        indexBuffer{indexBuffer} {
        buildAABB();
    }

    void Mesh::setSurfaceMaterial(const uint32 surfaceIndex, const std::shared_ptr<Material>& material) {
        assert([&]{return surfaceIndex < surfaces.size();}, "Invalid surface index");
        surfaces[surfaceIndex]->material = material;
        materials.insert(surfaces[surfaceIndex]->material);
    }

    void Mesh::upload(Window* window) {
        assert([&]{return window != nullptr; }, "Invalid window");
        assert([&]{return vertexBuffer == nullptr; }, "Already uploaded");
        const auto& vireo = window->getVireo();
        vertexBuffer = vireo->createBuffer(vireo::BufferType::VERTEX, sizeof(Vertex), vertices.size(), getName());
        indexBuffer = vireo->createBuffer(vireo::BufferType::INDEX, sizeof(uint32), indices.size(), getName());
        const auto allocator = vireo->createCommandAllocator(vireo::CommandType::GRAPHIC);
        const auto commandList = allocator->createCommandList();
        commandList->begin();
        commandList->upload(vertexBuffer, vertices.data());
        commandList->upload(indexBuffer, indices.data());
        commandList->end();
        window->getGraphicQueue()->submit({commandList});
        window->getGraphicQueue()->waitIdle();
        commandList->cleanup();
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
