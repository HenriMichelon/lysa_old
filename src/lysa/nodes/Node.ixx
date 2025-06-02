/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.node;

import std;
import lysa.global;
import lysa.input_event;

export namespace lysa {

    /**
     * Base class for all 3D nodes
     */
    class Node : public Object {
    public:
        friend class Viewport;

        //! Node type
        enum Type {
           ANIMATION_PLAYER,
           CAMERA,
           CHARACTER,
           COLLISION_AREA,
           COLLISION_OBJECT,
           DIRECTIONAL_LIGHT,
           ENVIRONMENT,
           KINEMATIC_BODY,
           LIGHT,
           MESH_INSTANCE,
           NODE,
           OMNI_LIGHT,
           PHYSICS_BODY,
           RAYCAST,
           RIGID_BODY,
           SKYBOX,
           SPOT_LIGHT,
           STATIC_BODY,
           VIEWPORT
        };

        static constexpr auto TypeNames = std::array {
            L"AnimationPlayer",
            L"Camera",
            L"Character",
            L"CollisionArea",
            L"CollisionObject",
            L"DirectionalLight",
            L"Environment",
            L"KinematicBody",
            L"Light",
            L"MeshInstance",
            L"Node",
            L"OmniLight",
            L"PhysicsBody",
            L"RayCast",
            L"RigidBody",
            L"Skybox",
            L"SpotLight",
            L"StaticBody",
            L"Viewport"
        };

        /**
         * Creates a node by copying the transforms, process mode, type and name
         */
        Node(const Node &node);

        /**
         * Creates a new node at (0.0, 0.0, 0.0) without a parent
         */
        Node(const std::wstring &name = TypeNames[NODE], Type type = NODE);

        /**
          * Returns the unique id of the node
          */
        auto getId() const { return id; }

        /**
         * Called when a node is ready to initialize, before being added to the scene
         */
        virtual void onReady() {}

        /**
         * Called when a node is added to the scene
         */
        virtual void onEnterScene() {}

        /**
         * Called when a node is removed from the scene
         */
        virtual void onExitScene() {}

        /**
         * Called each frame after the physics have been updated and just before drawing the frame
         */
        virtual void onProcess(const float alpha) {}

        /**
         * Called just after the physics system has been updated (can be called multiple times if we have free time between frames)
         */
        virtual void onPhysicsProcess(const float delta) {}

        /**
         * Called on a keyboard, mouse or gamepad event
         */
        virtual bool onInput(InputEvent &inputEvent) { return false; }

        /**
         * Returns the local space transformation matrix
         */
        // const float4x4 &getTransformLocal() const { return localTransform; }

        void setTransformLocal(const float4x4 &transform);

        /**
         * Returns the world space transformation matrix
         */
        const float4x4& getTransformGlobal() const { return globalTransform; }
    
        /**
         * Transforms a local vector from this node's local space to world space.
         */
        // float3 toGlobal(const float3& local) const;
    
        /**
         * Transforms a world space vector to this node's local space.
         */
        // float3 toLocal(const float3& global) const;
    
        /**
        * Sets the local space position (relative to parent)
        */
        virtual void setPosition(const float3& position);

        /**
        * Sets the local space position (relative to parent)
        */
        virtual void setPosition(const float x, const float y, const float z) { setPosition(float3{x, y, z}); }

        /**
        * Returns the local space position (relative to parent)
        */
        float3 getPosition() const { return localTransform[3].xyz; }
    
        /**
         * Changes the node's position by the given offset vector in local space.
         */
        void translate(const float3& localOffset);
    
        /**
         * Changes the node's position by the given offset vector in local space.
         */
        void translate(float x, float y, float z);

        /**
         * Sets the world space position
         */
        // virtual void setPositionGlobal(const float3& position);
    
        /**
         * Returns the world space position
         */
        float3 getPositionGlobal() const { return globalTransform[3].xyz; }
    
        /**
         * Rotates the local transformation
         */
        // void rotate(const quaternion& quaternion);
    
        /**
         * Interpolate the local transformation
         */
        // void rotateTowards(const quaternion& targetRotation, float maxAngle);
    
        /**
         * Rotates the local transformation around the X axis by angle in radians.
         */
        void rotateX(float angle);
    
        /**
         * Rotates the local transformation around the Y axis by angle in radians.
         */
        void rotateY(float angle);
    
        /**
         * Rotates the local transformation around the Z axis by angle in radians.
         */
        void rotateZ(float angle);

        /**
         * Returns the rotation of the local transformation
         */
        quaternion getRotationQuaternion() const;

        /**
         * Rotates the local transformation
         */
        // void rotate(quat quater);
    
        /**
         * Scale the local transformation
         */
        void scale(float scale);

        /**
         * Sets the local transformation
         */
        // void setRotation(const quaternion& quat);
    
        /**
         * Sets the world transformation
         */
        // void setRotationGlobal(const quaternion& quat);
    
        /**
         * Sets the local transformation
         */
        // void setRotation(const float3& rot);
    
        /**
         * Sets the X axis rotation of the local transformation by angle in radians.
         */
        // void setRotationX(float angle);
    
        /**
         * Sets the Y axis rotation of the local transformation by angle in radians.
         */
        // void setRotationY(float angle);
    
        /**
         * Sets the Z axis rotation of the local transformation by angle in radians.
         */
        // void setRotationZ(float angle);
    
        /**
         * Returns the rotation of the local transformation, in euler angles in radians
         */
        // float3 getRotation() const;
    
        /**
         * Returns the rotation of the world transformation, in euler angles in radians
         */
        // float3 getRotationGlobal() const;
    
        /**
         * Returns the rotation of the world transformation
         */
        // quaternion getRotationQuaternionGlobal() const;
    
        /**
         * Returns the X axis rotation of the local transformation
         */
        // float getRotationX() const { return getRotation().x; }
    
        /**
         * Returns the Y axis rotation of the local transformation
         */
        // float getRotationY() const { return getRotation().y; }
    
        /**
         * Returns the Z axis rotation of the local transformation
         */
        // float getRotationZ() const { return getRotation().z; }
    
        /**
         * Scales part of the local transformation.
         */
        // virtual void setScale(const float3& scale);
    
        /**
         * Scales part of the local transformation with the same value on each axis
         */
        // void setScale(float scale);
    
        /**
         * Returns the scale part of the local transformation.
         */
        // float3 getScale() const;
    
        /**
         * Returns the scale part of the global transformation.
         */
        // float3 getScaleGlobal() const;


        /**
         * Returns the node's parent in the scene tree
         */
        auto* getParent() const { return parent; }

        /**
         * Adds a child node.<br>
         * Nodes can have any number of children, but a child can have only one parent.<br>
         * The node will be added to the scene at the start of the next frame.
         * @param child the node to add
         * @param async if `true` and the node have children all the nodes will be added in batch mode.
         * Be careful to set the visibility of the nodes to `false`or they will appear slowly in the scene.
         */
        bool addChild(std::shared_ptr<Node> child, bool async = false);

        /**
         * Removes a child node. The node, along with its children **can** be deleted depending on their reference counter.<br>
         * Use the iterator version in a for-each loop.<br>
         * The node will be removed from the scene at the start of the next frame.
         * @param child the node to remove
         * @param async if `true` and the node have children all the nodes will be removed in batch mode.
         * Be careful to set the visibility of the nodes to `false` or they will disappear slowly from the scene.
         */
        bool removeChild(const std::shared_ptr<Node>& child, bool async = false);

        /**
         * Removes all children nodes. The nodes, along with their children **can** be deleted depending on their reference counters.
         * The nodes will be removed from the scene at the start of the next frame.
         * @param async if `true` and the nodes will be removed in batch mode.
         * Be careful to set the visibility of the nodes to `false` or they will disappear slowly from the scene.
         */
        void removeAllChildren(bool async = false);

        /**
         * Returns true if the node have this child
         */
        bool haveChild(const std:: shared_ptr<Node>& child, bool recursive) const;

        /**
        * Returns the child node by is name. Not recursive
        */
        template <typename T = Node>
        std::shared_ptr<T> getChild(const std::wstring& name) const {
            const auto it = std::find_if(children.begin(),
                                         children.end(),
                                         [name](const std::shared_ptr<Node>& elem) {
                                             return elem->name == name;
                                         });
            return it == children.end() ? nullptr : dynamic_pointer_cast<T>(*it);
        }

        /**
        * Returns the child node by its relative path (does not start with '/')
        */
        template <typename T = Node>
        std::shared_ptr<T> getChildByPath(const std::wstring& path) const {
            const size_t pos = path.find('/');
            if (pos != std::string::npos) {
                const auto child = getChild<Node>(path.substr(0, pos));
                if (child != nullptr) {
                    return child->template getChildByPath<T>(path.substr(pos + 1));
                }
                return nullptr;
            }
            return getChild<T>(path);
        }

        /**
        * Finds the first child by is name.
        */
        template<typename T = Node>
        std::shared_ptr<T> findFirstChild(const std::wstring& name) const {
            for (const auto &node : children) {
                if (node->name == name) {
                    return dynamic_pointer_cast<T>(node);
                }
                if (const auto& found = node->template findFirstChild<T>(name)) {
                    return found;
                }
            }
            return nullptr;
        }


        /**
         * Finds the first child by is type.
         */
        template <typename T>
        std::shared_ptr<T> findFirstChild(const bool recursive = true) const {
            for (const auto &node : children) {
                if (const auto& found = dynamic_pointer_cast<T>(node)) {
                    return found;
                }
                if (recursive) {
                    const auto result = node->template findFirstChild<T>(true);
                    if (result) {
                        return result;
                    }
                }
            }
            return nullptr;
        }

        /**
         * Finds all children by type
         */
        template <typename T>
        std::list<std::shared_ptr<T>> findAllChildren(const bool recursive = true) const {
            std::list<std::shared_ptr<T>> result;
            for (const auto &node : children) {
                if (const auto& found = dynamic_pointer_cast<T>(node)) {
                    result.push_back(found);
                }
                if (recursive) {
                    result.append_range(node->template findAllChildren<T>(true));
                }
            }
            return result;
        }

        /**
         * Finds all children by group
         */
        template <typename T = Node>
        std::list<std::shared_ptr<T>> findAllChildrenByGroup(const std::wstring& groupName, const bool recursive = true) const {
            std::list<std::shared_ptr<T>> result;
            for (const auto &node : children) {
                if (isInGroup(groupName)) {
                    result.push_back(dynamic_pointer_cast<T>(node));
                }
                if (recursive) {
                    result.append_range(node->template findAllChildrenByGroup<T>(groupName, true));
                }
            }
            return result;
        }

        /**
        * Returns the immutable list of children nodes
        */
        const std::list<std::shared_ptr<Node>> &getChildren() const { return children; }

        /**
        * Returns a list of group names that the node has been added to.
        */
        const std::list<std::wstring>& getGroups() const { return groups; }

        /**
         * Adds the node to the group. Groups can be helpful to organize a subset of nodes, for example "enemies" or "stairs".
         */
        void addToGroup(const std::wstring &group) { groups.push_back(group); }

        /**
         * Removes the node from the given group. Does nothing if the node is not in the group
         */
        void removeFromGroup(const std::wstring &group) { groups.remove(group); }

        /**
         * Returns true if this node has been added to the given group
         */
        auto isInGroup(const std::wstring& group) const { return std::ranges::find(groups, group) != groups.end(); }

        /**
         * Changes the node's processing behavior.
         */
        void setProcessMode(const ProcessMode mode) { processMode = mode; }

        /**
         * Returns true if the node is processed and receive input callbacks
         */
        bool isProcessed() const;

        /**
         * Returns the node type
         */
        auto getType() const { return type; }

        /**
         * Returns the attached rendering window or `nullptr` if the node is not attached to a window.
         */
        auto getViewport() const { return viewport; }

        /**
         * Returns the node name
         */
        const std::wstring &getName() const { return name; }

        /**
         * Returns the node path
         */
        std::wstring getPath() const;

        /**
        * Sets a property by its name and value.
        * Currently, not all properties in all node classes are supported.
        */
        virtual void setProperty(const std::string &property, const std::string &value);

        /**
         * Recursively prints the node tree in the log system
         */
        void printTree(int tab = 0) const;

        /**
         * Duplicates a node. Warning : not implemented on all node types, check documentation for the node type before using it.
         */
        std::shared_ptr<Node> duplicate() const;


        ~Node() override = default;
    
    protected:
        float4x4 localTransform{};
        float4x4 globalTransform{};

        virtual std::shared_ptr<Node> duplicateInstance() const;

        virtual void updateGlobalTransform();

        virtual void ready(Viewport* viewport);

        virtual void physicsProcess(float delta);

        virtual void process(float alpha);

        virtual void enterScene() { onEnterScene(); }

        virtual void exitScene() { onExitScene(); }

        void setUpdated() { updated = framesInFlight; }

    private:
        friend class Window;
        friend class Viewport;

        static  unique_id                currentId;
        unique_id                        id;
        Type                             type;
        std::wstring                     name;
        Viewport*                        viewport{nullptr};
        Node*                            parent{nullptr};
        std::list<std::shared_ptr<Node>> children;
        std::list<std::wstring>          groups;
        ProcessMode                      processMode{ProcessMode::INHERIT};

        friend class Scene;
        uint32   updated{0};
        uint32   framesInFlight{0};

        auto isUpdated() const { return updated > 0;}
    };

}
