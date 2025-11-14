/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.loader;

import std;
import lysa.nodes.node;

export namespace lysa {
    /**
     * Centralized loader for scene trees and external resources.
     *
     * The Loader resolves a file path from an URI and
     * builds a tree of Node instances from supported formats:
     *  - JSON scene descriptions (engine-specific)
     *  - glTF scenes
     *  - ZRes packaged resources
     *
     * The loader optionally caches root nodes to allow fast re-use across
     * the application lifetime. Access is thread-safe via an internal mutex.
     *
     * Notes:
     *  - All entry points are static; the class is used as a singleton-like
     *    utility (no instance needed).
     */
    class Loader {
    public:
        /**
         * Loads a JSON, glTF or ZRes file and returns the constructed node tree.
         *
         * The template parameter allows retrieving the result as a more specific
         * Node-derived type when the file describes such a root. If `usecache` is
         * `true` and the path was previously loaded, the cached root is returned.
         *
         * @tparam T Node subtype to cast the root to (defaults to Node).
         * @param filepath URI to the JSON/glTF/ZRes file
         * @param usecache When true, store and re-use the loaded root from a global cache.
         * @return Shared pointer to the loaded root node (possibly from cache).
         */
        template<typename T = Node>
        static std::shared_ptr<T> load(const std::string& filepath, bool usecache = false) {
            if (usecache) {
                auto lock = std::lock_guard(resourcesMutex);
                if (resources.contains(filepath)) {
                    // INFO("re-using resources", filepath);
                    return std::dynamic_pointer_cast<T>(resources[filepath]);
                }
            }
            const auto rootNode = make_shared<T>(filepath);
            load(rootNode, filepath, usecache);
            return rootNode;
        }

        /**
         * Searches all cached resource trees and returns the first node that
         * matches the provided name and type.
         *
         * @tparam T Node subtype used for the search and cast.
         * @param nodename Name to match against children of cached roots.
         * @return First matching node, or nullptr if none was found.
         */
        template<typename T = Node>
        static std::shared_ptr<T> findFirst(const std::string& nodename) {
            for (const auto& cachedResources : resources) {
                const auto& tree = cachedResources.second;
                auto node = tree->findFirstChild<T>(nodename);
                if (node) {
                    return dynamic_pointer_cast<T>(node);
                }
            }
            return nullptr;
        }

        /**
         * Clears the global resources cache.
         *
         * All previously cached root nodes become eligible for destruction once
         * no other shared references exist.
         */
        static void clearCache();

        /**
         * Intermediate description of a node parsed from a JSON scene file.
         *
         * This structure preserves declaration order of properties and children
         * to allow deterministic instantiation and to stay close to the source
         * file layout.
         */
        struct SceneNode {
            /** Optional unique identifier for referencing between nodes. */
            std::string id{};
            /** True if this node represents a reusable resource entry. */
            bool isResource{false};
            /** True if the node class is provided by user code (custom). */
            bool isCustom{false};
            /** True if this node was included from another file. */
            bool isIncluded{false};

            /** Fully qualified class/type name used to instantiate the node. */
            std::string clazz{};
            /** Single child (for linear chains) when applicable. */
            std::shared_ptr<SceneNode> child{nullptr};
            /** Ordered list of children nodes. */
            std::vector<SceneNode>  children{};
            /**
             * Ordered list of key/value properties.
             * Using a vector of pairs preserves JSON declaration order.
             */
            std::vector<std::pair<std::string, std::string>> properties{};
            // using a std::vector of pairs to preserve JSON declaration order

            /** Resource identifier (name or URI) associated with this node. */
            std::string resource{};
            /** Resolved path to the resource on disk or inside a package. */
            std::string resourcePath{};
            /** Resource type hint (e.g., mesh, texture). */
            std::string resourceType{};
            /** When true, indicates the instance must be duplicated at use. */
            bool needDuplicate{false};
        };

    private:
        /** Global map of file path to loaded root node (cache). */
        static inline std::map<std::string, std::shared_ptr<Node>> resources;
        /** Guards access to the resources cache for thread-safety. */
        static std::mutex resourcesMutex;

        /**
         * Internal entry that performs the actual loading/parsing and optionally
         * stores the resulting tree in the cache.
         */
        static void load(const std::shared_ptr<Node>&rootNode, const std::string& filepath, bool usecache);

        /** Parses and attaches a scene file content to the provided root node. */
        static void loadScene(const std::shared_ptr<Node>&rootNode, const std::string &filepath);

        /** Reads a JSON scene description and returns a flat list of SceneNode. */
        static std::vector<SceneNode> loadSceneDescriptionFromJSON(const std::string &filepath);

        /**
         * Instantiates and attaches a SceneNode to the runtime node tree, wiring
         * parent/child relationships and resolving resources as needed.
         */
        static void addNode(Node* parent,
                            std::map<std::string, std::shared_ptr<Node>>& nodeTree,
                            std::map<std::string, SceneNode>& sceneTree,
                            const SceneNode& nodeDesc);
    };

}
