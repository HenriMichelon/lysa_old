/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.resources.shape;

namespace lysa {

    Shape::Shape(const std::wstring &resName):
        Resource{resName} {
    }

    AABBShape::AABBShape(const std::shared_ptr<Node> &node, const std::wstring &resName) : AABBShape{*node, resName} {}

}
