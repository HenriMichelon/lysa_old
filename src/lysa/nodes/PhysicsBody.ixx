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
#ifdef PHYSIC_ENGINE_PHYSX
#include <PxPhysicsAPI.h>
#endif
export module lysa.nodes.physics_body;

import lysa.viewport;
import lysa.nodes.collision_object;
import lysa.resources.shape;

export namespace lysa {

    /**
     * Base class for 3D game objects affected by physics.
     */
    class PhysicsBody : public CollisionObject {
    public:

        void recreateBody();

        void setProperty(const std::string &property, const std::string &value) override;

        ~PhysicsBody() override = default;

    protected:
        void setShape(const std::shared_ptr<Shape> &shape);

        virtual void createBody(const std::shared_ptr<Shape> &shape);

        void attachToViewport(Viewport* viewport) override;

#ifdef PHYSIC_ENGINE_JOLT
        JPH::EMotionType motionType;
        JPH::Shape* joltShape{nullptr};
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
#ifdef PHYSIC_ENGINE_PHYSX
        physx::PxActorType::Enum actorType;
        PhysicsBody(const std::shared_ptr<Shape>& shape,
                    collision_layer layer,
                    physx::PxActorType::Enum actorType,
                    const std::wstring& name = TypeNames[PHYSICS_BODY],
                    Type type = PHYSICS_BODY);
        PhysicsBody(collision_layer layer,
                    physx::PxActorType::Enum actorType,
                    const std::wstring& name = TypeNames[PHYSICS_BODY],
                    Type type = PHYSICS_BODY);
        void createShape() override;
#endif
    };

}
