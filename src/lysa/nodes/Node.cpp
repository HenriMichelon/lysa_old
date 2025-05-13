/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.node;

namespace lysa {

    id_t Node::currentId = 0;

    Node::Node(const std::wstring &name, const Type type):
        id{++currentId},
        type{type},
        name{name} {

    }

    std::wstring Node::sanitizeName(const std::wstring &name) {
        auto newName = name;
        std::ranges::replace(newName, '/', '_');
        std::ranges::replace(newName, ':', '_');
        return newName;
    }

    void Node::ready() {
        onReady();
    }

}
