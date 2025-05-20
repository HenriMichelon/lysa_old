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

    // MeshInstance::MeshInstance(const MeshInstance & original):
    //     Node{original.name, MESH_INSTANCE},
    //     mesh{original.mesh},
    //     worldAABB{original.worldAABB},
    //     outlined{original.outlined},
    //     outlineMaterial{original.outlineMaterial} {
    //     log("mesh instance copy");
    // }

    std::shared_ptr<Node> MeshInstance::duplicateInstance() const {
        return std::make_shared<MeshInstance>(*this);
    }

    void MeshInstance::updateGlobalTransform() {
        Node::updateGlobalTransform() ;
        worldAABB = mesh->getAABB().toGlobal(globalTransform) ;
    }

}
