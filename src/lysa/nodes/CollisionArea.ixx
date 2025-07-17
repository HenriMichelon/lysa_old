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
#endif
export module lysa.nodes.collision_area;

import lysa.viewport;
import lysa.nodes.collision_object;
import lysa.resources.shape;

export namespace lysa {

    /**
     * Collision sensor that reports contacts with other bodies.
     */
    class CollisionArea : public CollisionObject {
    public:
        /**
         * Creates a CollisionArea using the given geometric `shape`
         * to detect collision with bodies having a layer in the `mask` value.
         * @param shape The collision shape
         * @param layer The collision layer
         * @param name The node name
         */
        CollisionArea(const std::shared_ptr<Shape>& shape,
                      collision_layer layer,
                      const std::wstring& name = TypeNames[COLLISION_AREA]);

        /**
         * Creates a CollisionArea without a collision shape
         */
        explicit CollisionArea(const std::wstring& name = TypeNames[COLLISION_AREA]);

        /**
         * Sets the collision shape of the area
         */
        void setShape(const std::shared_ptr<Shape> &shape);

        ~CollisionArea() override = default;

        void setProperty(const std::string &property, const std::string &value) override;

    protected:
        std::shared_ptr<Node> duplicateInstance() const override;

        virtual void createBody();

        void attachToViewport(Viewport* viewport) override;
#ifdef PHYSIC_ENGINE_JOLT
        JPH::Shape* joltShape{nullptr};
#endif
#ifdef PHYSIC_ENGINE_PHYSX
        void createShape() override;
#endif
    };

}
