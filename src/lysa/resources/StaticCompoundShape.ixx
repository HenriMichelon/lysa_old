/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.static_compound_shape;

import lysa.global;
import lysa.resources.shape;

export namespace lysa {

    /**
     * Sub shape composing a StaticCompoundShape
     */
    struct SubShape {
        /**
         * The geometry shape
         */
        std::shared_ptr<Shape> shape;

        /**
         * Local space position
         */
        float3 position{0.0f};

        /**
         * Local space rotation
         */
        quaternion rotation{quaternion::identity()};
    };

    /**
     * Collision shape composed by a collection of SubShape
     */
    class StaticCompoundShape : public Shape {
    public:
        /**
         * Creates a StaticCompoundShape using the `subshapes` collection of Shape
         */
        StaticCompoundShape(
            const std::vector<SubShape> &subshapes,
            const std::wstring &resName = L"StaticCompoundShape");
    };

}
