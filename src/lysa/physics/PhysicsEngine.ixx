/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.physics.engine;

import std;
import lysa.enums;
import lysa.math;
import lysa.physics.configuration;
import lysa.physics.physics_material;
import lysa.renderers.debug;

export namespace lysa {

    class PhysicsScene {
    public:
        virtual void update(float deltaTime) = 0;

        virtual void debug(DebugRenderer& debugRenderer) = 0;

        /**
        * Returns the physics system gravity
        */
        virtual float3 getGravity() const = 0;

        virtual ~PhysicsScene() = default;
    };

    class PhysicsEngine {
    public:
        static std::unique_ptr<PhysicsEngine> create(const PhysicsConfiguration& config);

        virtual std::unique_ptr<PhysicsScene> createScene()  = 0;

        virtual PhysicsMaterial* createMaterial(
            float friction = 0.0f,
            float restitution = 0.0f) const = 0;

        virtual void setRestitutionCombineMode(PhysicsMaterial* physicsMaterial, CombineMode combineMode) const = 0;

        virtual PhysicsMaterial* duplicateMaterial(const PhysicsMaterial* orig) const = 0;

        virtual ~PhysicsEngine() = default;
    };

}