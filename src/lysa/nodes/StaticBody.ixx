/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.static_body;

import lysa.nodes.physics_body;
import lysa.resources.shape;

export namespace lysa {

    /**
     * %A 3D physics body that can't be moved by external forces or velocity.
     */
    class StaticBody : public PhysicsBody {
    public:
        /**
         * Creates a StaticBody with a given collision `shape`, 
         * belonging to the `layer` layers.
         */
        StaticBody(const std::shared_ptr<Shape>& shape,
                   collision_layer layer,
                   const std::wstring& name = TypeNames[STATIC_BODY]);

        /**
         * Creates a StaticBody without a collision shape`
         * belonging to the `layer` layers
         */
        explicit StaticBody(collision_layer layer,
                            const std::wstring &name = TypeNames[STATIC_BODY]);

        /**
        * Creates a StaticBody without a collision shape`
        */
        explicit StaticBody(const std::wstring &name = TypeNames[STATIC_BODY]);

        ~StaticBody() override = default;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;
    };

}
