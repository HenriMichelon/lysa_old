/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.convex_hull_shape;

import std;
import lysa.math;
import lysa.nodes.node;
import lysa.nodes.mesh_instance;
import lysa.resources.shape;
import lysa.resources.mesh;

export namespace lysa {

    /**
     * %A convex hull collision shape
     */
    class ConvexHullShape : public Shape {
    public:
        /**
         * Creates a ConvexHullShape using the vertices of the Mesh of the first MeshInstance found in the `node` tree.
         * Uses the local transform of the node when creating the shape.
         */
        ConvexHullShape(const std::shared_ptr<Node> &node, const std::wstring &resName = L"ConvexHullShape");

        /**
         * Creates a ConvexHullShape using the vertices of the Mesh
         */
        ConvexHullShape(const std::shared_ptr<Mesh> &mesh, const std::wstring &resName = L"ConvexHullShape");

        /**
         * Creates a ConvexHullShape using a list of vertices
         */
        ConvexHullShape(const std::vector<float3>& points, const std::wstring &resName);

        std::shared_ptr<Resource> duplicate()  const override;

    private:
        std::vector<float3> points;

        void tryCreateShape(const std::shared_ptr<Node> &node);

        void createShape(const std::shared_ptr<MeshInstance>& meshInstance);

        void createShape(const std::shared_ptr<Mesh> &mesh);

        void createShape();
    };

}
