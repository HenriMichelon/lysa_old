/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.shape;

import lysa.application;

namespace lysa {

    Shape::Shape(const PhysicsMaterial* material, const std::wstring &resName):
        Resource{resName}{
        this->material = material ?
            Application::getPhysicsEngine().duplicateMaterial((material)):
            Application::getPhysicsEngine().createMaterial();
    }

    AABBShape::AABBShape(
        const std::shared_ptr<Node> &node,
        const PhysicsMaterial* material,
        const std::wstring &resName) : AABBShape{*node, material, resName} {}

}
