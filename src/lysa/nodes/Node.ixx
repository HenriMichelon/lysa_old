/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.node;

import std;
import lysa.object;
import lysa.types;

export namespace lysa {

    /**
     * Base class for all 3D nodes
     */
    class Node : public Object {
    public:
        //! Node type
        enum Type {
            NODE,
        };

        static constexpr auto TypeNames = std::array {
            L"Node",
        };

        /**
         * Creates a node by copying the transforms, process mode, type and name
         */
        // Node(const Node &node);

        /**
         * Creates a new node at (0.0, 0.0, 0.0) without a parent
         */
        Node(const std::wstring &name = TypeNames[NODE], Type type = NODE);

        /**
         * Called when a node is ready to initialize, before being added to the scene
         */
        virtual void onReady() {}

        /**
         * Called when a node is added to the scene
         */
        // virtual void onEnterScene() {}

        /**
         * Called when a node is removed from the scene
         */
        // virtual void onExitScene() {}

        /**
         * Called each frame after the physics have been updated and just before drawing the frame
         */
        // virtual void onProcess(const float alpha) {}

        /**
         * Called just after the physics system has been updated (can be called multiple times if we have free time between frames)
         */
        // virtual void onPhysicsProcess(const float delta) {}

    //     /**
    //      * Returns the local space transformation matrix
    //      */
    //     const glm::mat4 &getTransformLocal() const { return localTransform; }
    //
    //     /**
    //      * Returns the world space transformation matrix
    //      */
    //     const glm::mat4& getTransformGlobal() const { return globalTransform; }
    //
    //     /**
    //      * Transforms a local vector from this node's local space to world space.
    //      */
    //     glm::vec3 toGlobal(const glm::vec3& local) const;
    //
    //     /**
    //      * Transforms a world space vector to this node's local space.
    //      */
    //     glm::vec3 toLocal(const glm::vec3& global) const;
    //
    //     /**
    //     * Sets the local space position (relative to parent)
    //     */
    //     virtual void setPosition(const glm::vec3& position);
    //
    //     /**
    //     * Returns the local space position (relative to parent)
    //     */
    //     glm::vec3 getPosition() const { return localTransform[3]; }
    //
    //     /**
    //      * Changes the node's position by the given offset vector in local space.
    //      */
    //     void translate(const glm::vec3& localOffset);
    //
    //     /**
    //      * Sets the world space position
    //      */
    //     virtual void setPositionGlobal(const glm::vec3& position);
    //
    //     /**
    //      * Returns the world space position
    //      */
    //     glm::vec3 getPositionGlobal() const { return globalTransform[3]; }
    //
    //     /**
    //      * Rotates the local transformation
    //      */
    //     void rotate(const glm::quat& quaternion);
    //
    //     /**
    //      * Interpolate the local transformation
    //      */
    //     void rotateTowards(const glm::quat& targetRotation, float maxAngle);
    //
    //     /**
    //      * Rotates the local transformation around the X axis by angle in radians.
    //      */
    //     void rotateX(float angle);
    //
    //     /**
    //      * Rotates the local transformation around the Y axis by angle in radians.
    //      */
    //     void rotateY(float angle);
    //
    //     /**
    //      * Rotates the local transformation around the Z axis by angle in radians.
    //      */
    //     void rotateZ(float angle);
    //
    //     /**
    //      * Rotates the local transformation
    //      */
    //     // void rotate(quat quater);
    //
    //     /**
    //      * Sets the local transformation
    //      */
    //     void setRotation(const glm::quat& quater);
    //
    //     /**
    //      * Sets the world transformation
    //      */
    //     void setRotationGlobal(const glm::quat& quater);
    //
    //     /**
    //      * Sets the local transformation
    //      */
    //     void setRotation(const glm::vec3& rot);
    //
    //     /**
    //      * Sets the X axis rotation of the local transformation by angle in radians.
    //      */
    //     void setRotationX(float angle);
    //
    //     /**
    //      * Sets the Y axis rotation of the local transformation by angle in radians.
    //      */
    //     void setRotationY(float angle);
    //
    //     /**
    //      * Sets the Z axis rotation of the local transformation by angle in radians.
    //      */
    //     void setRotationZ(float angle);
    //
    //     /**
    //      * Returns the rotation of the local transformation, in euler angles in radians
    //      */
    //     glm::vec3 getRotation() const;
    //
    //     /**
    //      * Returns the rotation of the world transformation, in euler angles in radians
    //      */
    //     glm::vec3 getRotationGlobal() const;
    //
    //     /**
    //      * Returns the rotation of the local transformation
    //      */
    //     glm::quat getRotationQuaternion() const;
    //
    //     /**
    //      * Returns the rotation of the world transformation
    //      */
    //     glm::quat getRotationQuaternionGlobal() const;
    //
    //     /**
    //      * Returns the X axis rotation of the local transformation
    //      */
    //     float getRotationX() const { return getRotation().x; }
    //
    //     /**
    //      * Returns the Y axis rotation of the local transformation
    //      */
    //     float getRotationY() const { return getRotation().y; }
    //
    //     /**
    //      * Returns the Z axis rotation of the local transformation
    //      */
    //     float getRotationZ() const { return getRotation().z; }
    //
    //     /**
    //      * Scales part of the local transformation.
    //      */
    //     virtual void setScale(const glm::vec3& scale);
    //
    //     /**
    //      * Scales part of the local transformation with the same value on each axis
    //      */
    //     void setScale(float scale);
    //
    //     /**
    //      * Returns the scale part of the local transformation.
    //      */
    //     glm::vec3 getScale() const;
    //
    //     /**
    //      * Returns the scale part of the global transformation.
    //      */
    //     glm::vec3 getScaleGlobal() const;
    //
    //
    //     /**
    //      * Returns the attached surface or `nullptr` if the node is not rendered in a surface.
    //      */
    //     auto getSurface() const { return surface; }
    //
    //     ~Node() override = default;
    //
    // protected:
    //     glm::mat4 localTransform{};
    //     glm::mat4 globalTransform{};
    //
    //     virtual void updateGlobalTransform();

    private:
        friend class Surface;

        static  id_t                currentId;
        id_t                        id;
        Type                        type;
        std::wstring                name;
        const Surface*              surface{nullptr};
        Node*                       parent{nullptr};
        std::vector<std::shared_ptr<Node>>  children;

        static std::wstring sanitizeName(const std::wstring &name);

        virtual void ready(const Surface* surface);

    };

}
