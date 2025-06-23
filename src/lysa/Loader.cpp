/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <json.hpp>
module lysa.loader;

import lysa.assets_pack;
import lysa.global;
import lysa.type_registry;
import lysa.virtual_fs;

namespace lysa {

    std::mutex Loader::resourcesMutex;

    void Loader::load(const std::shared_ptr<Node>&rootNode, const std::wstring& filepath, const bool usecache) {
        if (filepath.ends_with(L".json")) {
            loadScene(rootNode, filepath);
            return;
        }
        if (filepath.ends_with(L".assets")) {
            AssetsPack::load(*rootNode, filepath);
        } else {
            throw Exception("Loader : unsupported file format for", lysa::to_string(filepath));
        }
        if (usecache) {
            auto lock = std::lock_guard(resourcesMutex);
            resources[filepath] = rootNode;
        }
    }

    void Loader::clearCache() {
        resources.clear();
    }

    void Loader::addNode(Node *parent,
                         std::map<std::string, std::shared_ptr<Node>> &nodeTree,
                         std::map<std::string, SceneNode> &sceneTree,
                         const SceneNode &nodeDesc) {
        constexpr auto log_name{"Scene loader :"};
        if (nodeTree.contains(nodeDesc.id)) {
            throw Exception(log_name, "Node with id", nodeDesc.id, "already exists in the scene tree");
        }
        // DEBUG("Loader::addNode ", nodeDesc.id);
        sceneTree[nodeDesc.id] = nodeDesc;
        std::shared_ptr<Node> node;
        if (nodeDesc.isResource) {
            if (nodeDesc.resourceType == "resource") {
                // the model is in a glTF/ZScene file
                node = load(lysa::to_wstring(nodeDesc.resource), true);
                node->setName(lysa::to_wstring(nodeDesc.id));
            } else if (nodeDesc.resourceType == "mesh") {
                // the model is part of another, already loaded, model
                if (nodeTree.contains(nodeDesc.resource)) {
                    // get the parent resource
                    const auto &resource = nodeTree[nodeDesc.resource];
                    // get the mesh node via the relative path
                    node = resource->getChildByPath(lysa::to_wstring(nodeDesc.resourcePath));
                    if (node == nullptr) {
                        resource->printTree();
                        throw Exception(log_name, "Mesh with path", nodeDesc.resourcePath, "not found");
                    }
                } else {
                    throw Exception(log_name, "Resource with id", nodeDesc.resource, "not found");
                }
            }
        } else {
            if (nodeDesc.clazz.empty() || nodeDesc.isCustom) {
                node = make_shared<Node>(lysa::to_wstring(nodeDesc.id));
            } else {
                // The node class is a registered class
                node = TypeRegistry::makeShared<Node>(nodeDesc.clazz);
                node->setName(lysa::to_wstring(nodeDesc.id));
            }
            node->setParent(parent);
            auto parentNode = node;
            auto childrenList = nodeDesc.children;
            if (nodeDesc.child != nullptr) {
                auto child = nodeTree[nodeDesc.child->id];
                if (child == nullptr) {
                    if (nodeDesc.child->id.contains(".mesh")) {
                        // child not found in current resources, try cached resources
                        static const std::regex pattern(R"(\.mesh)");
                        const std::string name{regex_replace(nodeDesc.child->id, pattern, "")};
                        child = findFirst(lysa::to_wstring(name));
                    }
                    if (child == nullptr) {
                        throw Exception(log_name, "Child node", nodeDesc.child->id, "not found");
                    }
                }
                if (nodeDesc.child->needDuplicate) {
                    // _LOG("nodeDesc.child->needDuplicate ", child->getName());
                    child = child->duplicate(true);
                }
                child->setPosition(FLOAT3ZERO);
                child->setRotation(QUATERNION_IDENTITY);
                // child->setScale(1.0f);
                if (child->getParent() != nullptr) {
                    child->getParent()->removeChild(child);
                }
                node->addChild(child);
                parentNode = child;
                childrenList = nodeDesc.child->children;
            }
            for (const auto &child : childrenList) {
                if (nodeTree.contains(child.id)) {
                    auto &childNode = nodeTree[child.id];
                    if (child.needDuplicate) {
                        // _LOG("Loader child.needDuplicate ", childNode->getName());
                        parentNode->addChild(childNode->duplicate());
                    } else {
                        // _LOG("Loader !child.needDuplicate ", childNode->getName());
                        if (childNode->getParent() != nullptr) {
                            childNode->getParent()->removeChild(childNode);
                        }
                        parentNode->addChild(childNode);
                    }
                } else {
                    // _LOG("Loader child addNode ", child.id);
                    addNode(parentNode.get(), nodeTree, sceneTree, child);
                }
            }
            for (const auto &prop : nodeDesc.properties) {
                node->setProperty(prop.first, prop.second);
            }

            node->setParent(nullptr);
            if (!nodeDesc.isIncluded) {
                parent->addChild(node);
            }
        }
        // cout << node->getName() << " -> " << nodeDesc.id << endl;
        nodeTree[nodeDesc.id] = node;
    }

    void Loader::loadScene(const std::shared_ptr<Node>&rootNode, const std::wstring &filepath) {
        // const auto tStart = chrono::high_resolution_clock::now();
        std::map<std::string, std::shared_ptr<Node>> nodeTree;
        std::map<std::string, SceneNode>        sceneTree;
        for (const auto &nodeDesc : loadSceneDescriptionFromJSON(filepath)) {
            addNode(rootNode.get(), nodeTree, sceneTree, nodeDesc);
        }
        // https://jrouwe.github.io/JoltPhysics/class_physics_system.html#ab3cd9f2562f0f051c032b3bc298d9604
        // app()._getPhysicsSystem().OptimizeBroadPhase();
        // const auto last_time = chrono::duration<float, milli>(chrono::high_resolution_clock::now() - tStart).count();
        // log("loadScene loading time ", to_string(last_time));
    }

    void from_json(const nlohmann::ordered_json &j, Loader::SceneNode &node) {
        j.at("id").get_to(node.id);
        node.isResource    = j.contains("resource");
        node.isCustom      = j.contains("custom");
        node.needDuplicate = j.contains("duplicate");
        if (node.isResource) {
            j.at("resource").get_to(node.resource);
            j.at("type").get_to(node.resourceType);
            if (j.contains("path"))
                j.at("path").get_to(node.resourcePath);
        } else {
            if (j.contains("class"))
                j.at("class").get_to(node.clazz);
            if (j.contains("properties")) {
                for (auto &[k, v] : j.at("properties").items()) {
                    node.properties.push_back({k, v});
                }
            }
            if (j.contains("child")) {
                node.child = std::make_shared<Loader::SceneNode>();
                j.at("child").get_to(*(node.child));
            }
            if (j.contains("children"))
                j.at("children").get_to(node.children);
        }
    }

    std::vector<Loader::SceneNode> Loader::loadSceneDescriptionFromJSON(const std::wstring &filepath) {
        std::vector<SceneNode> scene{};
        try {
            auto jsonData = nlohmann::ordered_json::parse(VirtualFS::openReadStream(filepath)); // parsing using ordered_json to preserver fields order
            if (jsonData.contains("includes")) {
                const std::vector<std::string> includes = jsonData["includes"];
                for (const auto &include : includes) {
                    std::vector<SceneNode> includeNodes = loadSceneDescriptionFromJSON(std::to_wstring(include));
                    for(auto& node : includeNodes) {
                        node.isIncluded = true;
                    }
                    scene.append_range(includeNodes);
                }
            }
            std::vector<SceneNode> nodes = jsonData["nodes"];
            scene.append_range(nodes);
        } catch (nlohmann::json::parse_error) {
            throw Exception("Error loading scene from JSON file ", lysa::to_string(filepath));
        }
        return scene;
    }
} // namespace z0
