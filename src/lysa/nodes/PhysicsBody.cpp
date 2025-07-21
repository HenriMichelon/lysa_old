/*
 * Copyright (c) 2024-present Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.nodes.physics_body;

import lysa.global;
import lysa.log;
import lysa.nodes.collision_object;
import lysa.nodes.mesh_instance;
import lysa.resources.shape;
import lysa.resources.convex_hull_shape;
import lysa.resources.mesh_shape;
import lysa.resources.static_compound_shape;

namespace lysa {

    void PhysicsBody::attachToViewport(Viewport* viewport) {
        CollisionObject::attachToViewport(viewport);
        createBody();
    }

    void PhysicsBody::recreateBody() {
        createBody();
    }

    void PhysicsBody::setProperty(const std::string &property, const std::string &value) {
        CollisionObject::setProperty(property, value);
        if (property == "shape") {
            // split shape class name from parameters
            const auto parts = split(value, ';');
            // we must have at least a class name
            if (parts.size() > 0) {
                if (parts.at(0) == "ConvexHullShape") {
                    if (parts.size() > 2) { throw Exception("Missing parameter for ConvexHullShape for ", getName()); }
                    // get the children who provide the mesh for the shape
                    const auto mesh = getChild(parts[1].data());
                    if (mesh == nullptr) { throw Exception("Child with pat h", parts[1].data(), " not found in", getName()); }
                    if (mesh->getType() != MESH_INSTANCE) { throw Exception("Child with path ", parts[1].data(), " not a MeshInstance in ", getName()); }
                    setShape(make_shared<ConvexHullShape>(mesh, nullptr, getName()));
                } else if (parts.at(0) == "BoxShape") {
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
                            make_shared<MeshShape>(meshInstance),
                            meshInstance->getPositionGlobal(),
                            meshInstance->getRotationGlobal()
                        });
                    }
                    setShape(make_shared<StaticCompoundShape>(subShapes));
                } else {
                    throw Exception("PhysicsBody : missing or invalid shape for ", getName());
                }
            }
        }
    }

}
