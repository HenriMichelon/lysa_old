/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.mesh_shape;

import lysa.global;

namespace lysa {

    MeshShape::MeshShape(
        const std::shared_ptr<Node> &node,
        PhysicsMaterial* material,
        const std::wstring &resName ):
        Shape{material, resName} {
        tryCreateShape(node);
    }

    MeshShape::MeshShape(
        const Node &node,
        PhysicsMaterial* material,
        const std::wstring &resName):
        Shape{material, resName} {
        const auto& meshInstance = node.findFirstChild<MeshInstance>();
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        } else {
            throw Exception("MeshShape : Node ", lysa::to_string(node.getName()), "does not have a MeshInstance child");
        }
    }

    void MeshShape::tryCreateShape(
        const std::shared_ptr<Node>& node) {
        auto meshInstance = std::dynamic_pointer_cast<MeshInstance>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance != nullptr) {
            createShape(meshInstance);
        }
    }

}
