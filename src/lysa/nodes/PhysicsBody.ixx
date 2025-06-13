/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/EActivation.h>
#endif
export module lysa.nodes.physics_body;

import lysa.nodes.collision_object;
import lysa.resources.shape;

export namespace lysa {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody : public CollisionObject {
    public:

        /**
         * Sets the linear velocity
         */
        virtual void setVelocity(const float3& velocity);

        /**
         *
         * Sets the gravity multiplier (set to 0.0f to disable gravity).
         */
        virtual void setGravityFactor(float factor);

        /**
        * Sets the coefficient of restitution
        * (the ratio of the relative velocity of separation after collision to the relative velocity of approach before collision)
        */
        void setBounce(float value) const;

        /**
         * Sets the body's mass.
         */
        void setMass(float value) const;

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

        void recreateBody();

        ~PhysicsBody() override = default;

    protected:
        /**
         * Sets a new collision shape, recreates the body in the physic system
         */
        void setShape(const std::shared_ptr<Shape> &shape);

#ifdef PHYSIC_ENGINE_JOLT
        JPH::EMotionType motionType;
        PhysicsBody(const std::shared_ptr<Shape> &shape,
                    collision_layer layer,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const std::wstring& name= TypeNames[PHYSICS_BODY],
                    Type type = PHYSICS_BODY);

        PhysicsBody(collision_layer layer,
                    JPH::EActivation activationMode,
                    JPH::EMotionType motionType,
                    const std::wstring& name = TypeNames[PHYSICS_BODY],
                    Type type = PHYSICS_BODY);
#endif
    };

}
