/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.character;

namespace lysa {

    Character::Character(const float height,
                         const float radius,
                         const collision_layer layer,
                         const std::wstring& name):
        CollisionObject(layer,
                        name,
                        CHARACTER) {
        setShape(height, radius);
    }

    void Character::enterScene() {
        setPositionAndRotation();
        Node::enterScene();
    }

    void Character::resume() {
        setPositionAndRotation();
    }

    void Character::setVisible(const bool visible) {
        if (visible != isVisible() && visible) {
            setPositionAndRotation();
        }
        Node::setVisible(visible);
    }

}