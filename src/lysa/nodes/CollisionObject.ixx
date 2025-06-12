/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.collision_object;

import lysa.math;
import lysa.signal;
import lysa.nodes.node;
import lysa.resources.shape;

export namespace lysa {

    /**
     * Base class for 3D physics objects.
     */
    class CollisionObject : public Node {
    public:

        /**
         * Signal called whenever a new contact point is detected and reports the first contact point in a CollisionObject::Collision<br>
         * For characters, called whenever the character collides with a body.
         */
        static inline const Signal::signal on_collision_starts = "on_collision_starts";

        /**
         * Signal called whenever a contact is detected that was also detected last update and reports the first contact point in a CollisionObject::Collision<br>
         * Never called for characters since on_collision_added is called during the whole contact.
         */
        static inline const Signal::signal on_collision_persists = "on_collision_persists";

        /**
         * Collision data for the CollisionObject::on_collision_starts and CollisionObject::on_collision_persists signal
         */
        struct Collision {
            /** World space position of the first contact point on the colliding `object` or, if the collision is detected by Character::getCollisions(), on the character */
            float3 position;
            /** Normal for this collision, the direction along which to move the `object` out of collision along the shortest path.  */
            float3 normal;
            /** Colliding object */
            CollisionObject *object;
        };

        /**
         * The physics layers this CollisionObject is in.
         */
        auto getCollisionLayer() const { return collisionLayer; }

        /**
         * Sets the collision layer
         */
        virtual void setCollisionLayer(uint32 layer);

        /**
         * Returns `true` if `obj` were in contact with the object during the last simulation step
         */
        bool wereInContact(const CollisionObject *obj) const;

        void setProperty(const std::string &property, const std::string &value) override;

        void setVisible(bool visible = true) override;

        CollisionObject(const CollisionObject&);

        ~CollisionObject() override;

    protected:
        bool updating{false};
        collision_layer collisionLayer;
        std::shared_ptr<Shape> shape{nullptr};

/*
        JPH::EActivation    activationMode;
        JPH::BodyInterface &bodyInterface;
        JPH::BodyID bodyId{JPH::BodyID::cInvalidBodyID};*/

        CollisionObject(const std::shared_ptr<Shape>&_shape,
                        uint32 layer,
                        const std::wstring&  name = TypeNames[COLLISION_OBJECT],
                        Type type = COLLISION_OBJECT);

        CollisionObject(uint32 layer,
                        const std::wstring &name = TypeNames[COLLISION_OBJECT],
                        Type type = COLLISION_OBJECT);

        virtual void setPositionAndRotation();

        //void setBodyId(JPH::BodyID id);

        //static CollisionObject *_getByBodyId(JPH::BodyID id);

        // void _updateTransform() override;

        // void _updateTransform(const mat4 &parentMatrix) override;

        //void releaseBodyId();


    private:
        //bool savedState{false};

    public:
        //void _update(float alpha) override;

        //void _onEnterScene() override;

        //void _onExitScene() override;

        //void _onPause() override;

        //void _onResume() override;

        //[[nodiscard]] inline auto _getBodyId() const { return bodyId; }

    };


}
