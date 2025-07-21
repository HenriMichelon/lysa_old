/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.mesh_shape;

import lysa.exception;
import lysa.nodes.mesh_instance;

namespace lysa {

    MeshShape::MeshShape(
        const std::shared_ptr<Node> &node,
        const PhysicsMaterial* material,
        const std::string &resName ):
        Shape{material, resName} {
        meshInstance = std::dynamic_pointer_cast<MeshInstance>(node);
        if (meshInstance == nullptr) {
            meshInstance = node->findFirstChild<MeshInstance>();
        }
        if (meshInstance == nullptr) {
            throw Exception("MeshShape : Node ", node->getName(), " does not have a MeshInstance child");
        }
    }

}
