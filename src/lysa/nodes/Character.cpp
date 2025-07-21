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
                         const std::string& name):
        CollisionObject(layer,
                        name,
                        CHARACTER),
        height{height},
        radius{radius} {
    }

    void Character::attachToViewport(Viewport* viewport) {
        CollisionObject::attachToViewport(viewport);
        setShape(height, radius);
    }

    void Character::enterScene() {
        CollisionObject::enterScene();
        setPositionAndRotation();
    }

    void Character::resume() {
        CollisionObject::resume();
        setPositionAndRotation();
    }

    void Character::setVisible(const bool visible) {
        if (visible != isVisible() && visible) {
            setPositionAndRotation();
        }
        Node::setVisible(visible);
    }

}