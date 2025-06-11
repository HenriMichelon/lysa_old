/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.mesh_instance;

import lysa.window;
import lysa.nodes.node;
import lysa.resources.mesh;

namespace lysa {

    MeshInstance::MeshInstance(const std::shared_ptr<Mesh>& mesh, const std::wstring &name):
        Node{name, MESH_INSTANCE},
        mesh{mesh} {
    }

    MeshInstanceData MeshInstance::getModelData() const {
        return {
            .transform = globalTransform,
            .aabbMin = worldAABB.min,
            .aabbMax = worldAABB.max,
            .visible = 1,
        };
    }

    std::shared_ptr<Node> MeshInstance::duplicateInstance() const {
        return std::make_shared<MeshInstance>(*this);
    }

    void MeshInstance::updateGlobalTransform() {
        Node::updateGlobalTransform() ;
        worldAABB = mesh->getAABB().toGlobal(globalTransform) ;
    }

}
