/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.node;

import std;
import glm;
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

        /**
         * Returns the attached surface or `nullptr` if the node is not rendered in a surface.
         */
        auto getSurface() const { return surface; }

        ~Node() override = default;

    private:
        friend class Surface;

        static  id_t                currentId;
        id_t                        id;
        Type                        type;
        std::wstring                name;
        const Surface*              surface{nullptr};

        static std::wstring sanitizeName(const std::wstring &name);

        virtual void ready(const Surface* surface);

    };

}
