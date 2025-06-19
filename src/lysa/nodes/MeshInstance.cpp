/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.mesh_instance;

import lysa.application;
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
            .visible = isVisible() ? 1u : 0u,
        };
    }

    const std::shared_ptr<Material>& MeshInstance::getSurfaceMaterial(const uint32 surfaceIndex) const {
        if (overrideMaterials.contains(surfaceIndex)) {
            return overrideMaterials.at(surfaceIndex);
        }
        return getMesh()->getSurfaces()[surfaceIndex]->material;
    }

    void MeshInstance::setSurfaceMaterial(const uint32 surfaceIndex, const std::shared_ptr<Material>& material) {
        const auto oldPipelineId = getMesh()->getSurfaceMaterial(surfaceIndex)->getPipelineId();
        getMesh()->setSurfaceMaterial(surfaceIndex, material);
        if (oldPipelineId != material->getPipelineId()) {
            const auto parent = getParent();
            if (getViewport() && parent) {
                const auto me = getSharedPtr();
                parent->removeChild(me);
                parent->addChild(me);
            }
        }
    }

    void MeshInstance::setSurfaceOverrideMaterial(const uint32 surfaceIndex, const std::shared_ptr<Material>& material) {
        if (material == nullptr) {
            overrideMaterials.erase(surfaceIndex);
        } else {
            overrideMaterials[surfaceIndex] = material;
        }
        if (material == nullptr || getMesh()->getSurfaceMaterial(surfaceIndex)->getPipelineId() != material->getPipelineId()) {
            const auto parent = getParent();
            if (getViewport() && parent) {
                const auto me = getSharedPtr();
                parent->removeChild(me);
                parent->addChild(me);
            }
        }
    }

    std::shared_ptr<Material> MeshInstance::getSurfaceOverrideMaterial(const uint32 surfaceIndex) {
        if (overrideMaterials.contains(surfaceIndex)) {
            return overrideMaterials.at(surfaceIndex);
        }
        return nullptr;
    }


    std::shared_ptr<Node> MeshInstance::duplicateInstance() const {
        return std::make_shared<MeshInstance>(*this);
    }

    void MeshInstance::updateGlobalTransform() {
        Node::updateGlobalTransform() ;
        worldAABB = mesh->getAABB().toGlobal(globalTransform) ;
    }

}
