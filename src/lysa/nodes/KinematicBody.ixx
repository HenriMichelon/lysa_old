/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.kinematic_body;

import lysa.nodes.physics_body;
import lysa.resources.shape;

export namespace lysa {

    /**
     * Physics body moved by velocities only, does not respond to forces.
     */
    class KinematicBody : public PhysicsBody {
    public:
        /**
         * Creates a KinematicBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit KinematicBody(const std::shared_ptr<Shape>& shape,
                               collision_layer layer = 0,
                               const std::string& name  = TypeNames[KINEMATIC_BODY]);

        /**
         * Creates a KinematicBody without a collision shape,
         */
        explicit KinematicBody(const std::string &name = TypeNames[KINEMATIC_BODY]);

        ~KinematicBody() override = default;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

    };

}
