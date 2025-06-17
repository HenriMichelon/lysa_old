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

        /**
         * Sets the density of the body
         */
        void setDensity(float density);

        /**
         * Returns the density of the body
         */
        auto getDensity() const { return density; }

        /**
         * Sets the linear velocity
         */
        virtual void setVelocity(const float3& velocity);

        /**
         * Sets the gravity multiplier (set to 0.0f to disable gravity).
         */
        virtual void setGravityFactor(float factor);

        /**
         * Sets the body's mass.
         */
        void setMass(float value);

        /**
         * Returns the linear velocity
         */
        virtual float3 getVelocity() const;

        /**
         * Add force (unit: N) at the center of mass for the next time step, will be reset after the next physics update
         */
        void applyForce(const float3& force) const;

        /**
         * Add force (unit: N) at `position` for the next time step, will be reset after the next physics update
         */
        void applyForce(const float3& force, const float3& position) const;

        void setProperty(const std::string &property, const std::string &value) override;

        ~RigidBody() override = default;

    protected:
        float density{1000.0f};

        std::shared_ptr<Node> duplicateInstance() const override;

#ifdef PHYSIC_ENGINE_PHYSX
        void createBody(const std::shared_ptr<Shape> &shape) override;
#endif

        void enterScene() override;

    };
}
