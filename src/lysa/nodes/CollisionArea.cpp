/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.collision_area;

import lysa.exception;
import lysa.global;
import lysa.nodes.mesh_instance;
import lysa.nodes.node;
import lysa.resources.mesh_shape;
import lysa.resources.shape;
import lysa.resources.static_compound_shape;

namespace lysa {

    CollisionArea::CollisionArea(const std::shared_ptr<Shape>& shape,
                                 const uint32 layer,
                                 const std::string& name):
        CollisionObject{shape, layer, name, COLLISION_AREA} {
        setShape(shape);
    }

    CollisionArea::CollisionArea(const std::string &name):
        CollisionObject{ 0, name, COLLISION_AREA} {
    }

    void CollisionArea::attachToViewport(Viewport* viewport) {
        CollisionObject::attachToViewport(viewport);
        createBody();
    }

    std::shared_ptr<Node> CollisionArea::duplicateInstance() const {
        return std::make_shared<CollisionArea>(*this);
    }

    void CollisionArea::setProperty(const std::string &property, const std::string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "shape") {
            // split shape class name from parameters
            const auto parts = split(value, ';');
            // we must have at least a class name
            if (parts.size() > 0) {
                if (parts.at(0) == "BoxShape") {
                    if (parts.size() < 2) { throw Exception("Missing parameter for BoxShape for ", getName()); }
                    setShape(make_shared<BoxShape>(to_float3(parts[1].data()), nullptr, getName()));
                } else if (parts.at(0) == "SphereShape") {
                    if (parts.size() < 2) { throw Exception("Missing parameter for SphereShape for ", getName()); }
                    setShape(make_shared<SphereShape>(std::stof(parts[1].data()), nullptr, getName()));
                } else if (parts.at(0) == "MeshShape") {
                    setShape(std::make_shared<MeshShape>(getSharedPtr()));
                } else if (parts.at(0) == "AABBShape") {
                    setShape(std::make_shared<AABBShape>(getSharedPtr()));
                } else if (parts.at(0) == "StaticCompoundShape") {
                    std::vector<SubShape> subShapes;
                    for (const auto &meshInstance : findAllChildren<MeshInstance>()) {
                        subShapes.push_back({
                            std::make_shared<MeshShape>(meshInstance),
                            meshInstance->getPositionGlobal(),
                            meshInstance->getRotationGlobal()
                        });
                    }
                    setShape(std::make_shared<StaticCompoundShape>(subShapes));
                } else {
                    throw Exception("CollisionArea : missing or invalid shape for ", getName());
                }
            }
        }
    }

}
