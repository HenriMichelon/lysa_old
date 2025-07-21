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
     * Singleton for loading external resources
     */
    class Loader {
    public:
        /**
         * Load a JSON, glTF or ZRes file
         * @param filepath path of the JSON/glTF/ZRes file, relative to the application path
         * @param usecache put loaded resources in the global resources cache
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

        static void clearCache();

        // Node description inside a JSON file
        struct SceneNode {
            std::string id{};
            bool isResource{false};
            bool isCustom{false};
            bool isIncluded{false};

            std::string clazz{};
            std::shared_ptr<SceneNode> child{nullptr};
            std::vector<SceneNode>  children{};
            std::vector<std::pair<std::string, std::string>> properties{};
            // using a std::vector of pairs to preserve JSON declaration order

            std::string resource{};
            std::string resourcePath{};
            std::string resourceType{};
            bool needDuplicate{false};
        };

    private:
        static inline std::map<std::string, std::shared_ptr<Node>> resources;
        static std::mutex resourcesMutex;

        static void load(const std::shared_ptr<Node>&rootNode, const std::string& filepath, bool usecache);

        static void loadScene(const std::shared_ptr<Node>&rootNode, const std::string &filepath);

        static std::vector<SceneNode> loadSceneDescriptionFromJSON(const std::string &filepath);

        static void addNode(Node* parent,
                            std::map<std::string, std::shared_ptr<Node>>& nodeTree,
                            std::map<std::string, SceneNode>& sceneTree,
                            const SceneNode& nodeDesc);
    };

}
