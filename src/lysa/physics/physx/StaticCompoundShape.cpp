/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <PxPhysicsAPI.h>
module lysa.resources.static_compound_shape;

namespace lysa {

    StaticCompoundShape::StaticCompoundShape(const std::vector<SubShape> &subshapes, const std::wstring &resName) :
        Shape{nullptr, resName} {
        for (const auto &subshape : subshapes) {
            this->subShapes.push_back(subshape);
        }
    }

}
