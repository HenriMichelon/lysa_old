/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.resources.shape;

import std;
import lysa.math;
import lysa.nodes.node;
import lysa.resources.resource;

export namespace lysa {

    using shape_handle = void*;

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    public:
        auto getShapeHandle() const { return shapeHandle; }

    protected:
        shape_handle shapeHandle{nullptr};

        explicit Shape(const std::wstring &resName);
    };

    /**
     * Box-shaped collision Shape
     */
    class BoxShape : public Shape {
    public:
        /**
         * Creates a BoxShape with the given extents
         */
        explicit BoxShape(const float3& extends, const std::wstring &resName = L"BoxShape");

        std::shared_ptr<Resource> duplicate() const override;

    private:
        const float3 extends;
    };

    /**
     * Sphere-shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        /**
         * Creates a SphereShape with the given radius
         */
        explicit SphereShape(float radius, const std::wstring &resName = L"SphereShape");

    private:
        explicit SphereShape(const std::wstring &resName) : Shape(resName) {}
    };

    /**
     * AABB-based collision Shape
     */
    class AABBShape : public Shape {
    public:
        /**
         * Creates an AABBShape for a given node
         */
        explicit AABBShape(const std::shared_ptr<Node> &node, const std::wstring &resName = L"AABBShape");

        /**
         * Creates an AABBShape for a given node
         */
        explicit AABBShape(const Node &node, const std::wstring &resName = L"AABBShape");

    private:
        explicit AABBShape(const std::wstring &resName) : Shape(resName) {}
    };

}
