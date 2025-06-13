/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.convex_hull_shape;

import lysa.resources.resource;

namespace lysa {

    ConvexHullShape::ConvexHullShape(const std::shared_ptr<Node> &node, const std::wstring &resName):
        Shape{resName} {
        tryCreateShape(node);
    }

    ConvexHullShape::ConvexHullShape(const std::shared_ptr<Mesh> &mesh, const std::wstring &resName):
        Shape{resName} {
        createShape(mesh);
    }

    void ConvexHullShape::tryCreateShape(const std::shared_ptr<Node> &node) {
        auto meshInstance = std::dynamic_pointer_cast<MeshInstance>(node);
        if (meshInstance == nullptr) {
            const auto& meshNode = node->findFirstChild<MeshInstance>();
            if (meshNode != nullptr) meshInstance = meshNode;
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }

    ConvexHullShape::ConvexHullShape(const std::vector<float3>& points, const std::wstring &resName):
    Shape{resName}, points{points} {
        createShape();
    }

    std::shared_ptr<Resource> ConvexHullShape::duplicate() const {
        return std::make_shared<ConvexHullShape>(points, getName());
    }

    void ConvexHullShape::createShape(const std::shared_ptr<MeshInstance>& meshInstance) {
        points.clear();
        const auto &transform = meshInstance->getTransform();
        for (const auto &vertex : meshInstance->getMesh()->getVertices()) {
            auto point = mul(float4{vertex.position, 1.0f}, transform);
            points.push_back(point.xyz);
        }
        createShape();
    }

    void ConvexHullShape::createShape(const std::shared_ptr<Mesh> &mesh) {
        points.clear();
        for (const auto &vertex : mesh->getVertices()) {
            points.push_back(vertex.position);
        }
        createShape();
    }

}
