/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.mesh;

import lysa.application;
import lysa.global;
import lysa.window;

namespace lysa {

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
        setUpdated();
    }

    void Mesh::upload() {
        auto& resources = Application::getResources();
        if (!isUploaded()) {
            vertexMemoryBloc = resources.getVertexArray().alloc(vertices.size());
        }
        resources.getVertexArray().write(vertexMemoryBloc, vertices.data());
        for (int i = 0; i < surfaces.size(); i++) {
        const auto& material = surfaces[i]->material;
            if (!material->isUploaded()) {
               material->upload();
            }
        }
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
