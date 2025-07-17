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
         * Returns the body's mass.
         */
        float getMass() const;

        /**
         * Returns the linear velocity
         */
        virtual float3 getVelocity() const;

        /**
         * Adds force at the center of mass for the next time step, will be reset after the next physics update
         */
        void addForce(const float3& force);

        /**
         * Adds force at `position` for the next time step, will be reset after the next physics update
         */
        void addForce(const float3& force, const float3& position);

        /**
         * Adds an impulse at the center of mass
         */
        void addImpulse(const float3& force);

        /**
         * Adds an impulse at `position`
         */
        void addImpulse(const float3& force, const float3& position);

        void setProperty(const std::string &property, const std::string &value) override;

        ~RigidBody() override = default;

#ifdef PHYSIC_ENGINE_JOLT
        void scale(float scale) override;
#endif

    protected:
        float gravityFactor{1.0f};
        float density{100.0f};
        float mass{-1.0f};

        std::shared_ptr<Node> duplicateInstance() const override;

        void createBody() override;

        void enterScene() override;

#ifdef PHYSIC_ENGINE_PHYSX
        bool forceApplied{false};
        void process(float delta) override;
        void createShape() override;
#endif

    };
}
