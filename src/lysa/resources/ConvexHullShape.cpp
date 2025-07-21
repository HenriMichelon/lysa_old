/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.convex_hull_shape;

import lysa.exception;
import lysa.resources.resource;

namespace lysa {

    ConvexHullShape::ConvexHullShape(
        const std::shared_ptr<Node> &node,
        const PhysicsMaterial* material,
        const std::string &resName):
        Shape{material, resName} {
        meshInstance = std::dynamic_pointer_cast<MeshInstance>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance == nullptr) {
            throw Exception("MeshShape : Node ", node->getName(), " does not have a MeshInstance child");
        }
    }

    std::shared_ptr<Resource> ConvexHullShape::duplicate() const {
        return std::make_shared<ConvexHullShape>(meshInstance, material, getName());
    }
    //
    // void ConvexHullShape::createShape(
    //     const std::shared_ptr<MeshInstance>& meshInstance) {
    //     points.clear();
    //     const auto &transform = meshInstance->getTransform();
    //     for (const auto &vertex : meshInstance->getMesh()->getVertices()) {
    //         auto point = mul(float4{vertex.position, 1.0f}, transform);
    //         points.push_back(point.xyz);
    //     }
    //     createShape();
    // }
    //
    // void ConvexHullShape::createShape(
    //     const std::shared_ptr<Mesh> &mesh) {
    //     points.clear();
    //     for (const auto &vertex : mesh->getVertices()) {
    //         points.push_back(vertex.position);
    //     }
    //     createShape();
    // }

}
