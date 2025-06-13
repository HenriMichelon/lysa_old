/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.rigid_body;

import lysa.nodes.physics_body;
import lysa.resources.shape;

export namespace lysa {
    /**
     * %A 3D physics body that is moved by a physics simulation, responds to forces.
     */
    class RigidBody : public PhysicsBody {
    public:
        /**
         * Creates a RigidBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit RigidBody(const std::shared_ptr<Shape>& shape,
                           collision_layer layer = 0,
                           const std::wstring& name  = TypeNames[RIGID_BODY]);

        /**
         * Creates a RigidBody without a collision shape,
         */
        explicit RigidBody(const std::wstring &name = TypeNames[RIGID_BODY]);

        ~RigidBody() override = default;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;
    };
}
