/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
module lysa.resources.mesh_shape;

import lysa.application;

namespace lysa {

    void MeshShape::createShape(
        const std::shared_ptr<MeshInstance>& meshInstance) {
        const auto & vertices  = meshInstance->getMesh()->getVertices();
        JPH::VertexList vertexList;
        vertexList.reserve(vertices.size());
        for (const auto &vertex : vertices) {
            vertexList.push_back(JPH::Float3{vertex.position.x, vertex.position.y, vertex.position.z});
        }
        const auto & indices = meshInstance->getMesh()->getIndices();
        JPH::IndexedTriangleList triangles;
        JPH::PhysicsMaterialList materials;
        const auto joltMaterial = reinterpret_cast<JPH::PhysicsMaterial*>(material);
        triangles.reserve(indices.size()/3);
        for (int i = 0; i < indices.size(); i += 3) {
            triangles.push_back({indices[i + 0], indices[i + 1], indices[i + 2]});
            materials.push_back(joltMaterial);
        }

        shapeSettings = new JPH::MeshShapeSettings(vertexList, triangles);
    }

}
