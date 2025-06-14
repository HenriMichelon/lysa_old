/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.shape;

namespace lysa {

    AABBShape::AABBShape(
        const std::shared_ptr<Node> &node,
        const std::shared_ptr<PhysicsMaterial>& material,
        const std::wstring &resName) : AABBShape{*node, material, resName} {}

}
