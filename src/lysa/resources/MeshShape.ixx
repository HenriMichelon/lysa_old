/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.mesh_shape;

import lysa.nodes.node;
import lysa.nodes.mesh_instance;
import lysa.physics.engine;
import lysa.resources.shape;

export namespace lysa {

    /**
     * %A mesh shape, consisting of triangles. *Must* only be used with a StaticBody (like a terrain for example)
     */
    class MeshShape : public Shape {
    public:
        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        MeshShape(
            const std::shared_ptr<Node> &node,
            const std::shared_ptr<PhysicsMaterial>& material = nullptr,
            const std::wstring &resName = L"MeshShape");

        /**
         * Creates a MeshShape using the triangles of the Mesh of first MeshInstance found in the `node` tree
         */
        MeshShape(
            const Node &node,
            const std::shared_ptr<PhysicsMaterial>& material = nullptr,
            const std::wstring &resName = L"MeshShape");

    protected:
        std::list<std::shared_ptr<PhysicsMaterial>> materials;

    private:
        void tryCreateShape(
            const std::shared_ptr<Node>& nodel);

        void createShape(
            const std::shared_ptr<MeshInstance>& meshInstance);
    };

}
