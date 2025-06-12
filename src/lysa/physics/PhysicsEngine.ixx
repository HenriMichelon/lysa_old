/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.physics.engine;

import std;
import lysa.math;
import lysa.signal;
import lysa.types;
import lysa.physics.configuration;

export namespace lysa {

    class PhysicsEngine {
    public:
        static std::unique_ptr<PhysicsEngine> create(const PhysicsConfiguration& config);

        virtual void update(float deltaTime) = 0;

        /**
        * Returns the physics system gravity
        */
        virtual float3 getGravity() const = 0;

        virtual ~PhysicsEngine() = default;
    };

}