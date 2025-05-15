/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.nodes.viewport;

import vireo;
import lysa.nodes.node;

export namespace lysa {

    /**
     * Node that define the scene renderer viewport and scissors position & size<br>
     * If no Viewport is defined in the scene, the whole rendering window is used
     */
    class Viewport : public Node {
    public:
        /**
         * Creates a Viewport
         * @param viewport Viewport
         * @param scissors Scissors
         * @param name node name
         */
        Viewport(const vireo::Viewport& viewport, const vireo::Rect& scissors, const std::wstring& name = TypeNames[VIEWPORT]) :
            Node{name}, viewport{viewport}, scissors{scissors} { }

        explicit Viewport(const std::wstring& name = TypeNames[VIEWPORT]) :
            Node{name} {}

        /**
         * Returns the viewport
         */
        const auto& getViewport() const { return viewport; }

        /**
         * Sets the viewport
         */
        void setViewport(const vireo::Viewport& viewport) { this->viewport = viewport; }

        /**
         * Returns the scissors rect
         */
        const auto& getScissors() const { return scissors; }

        /**
         * Sets the scissors rect
         */
        void setScissors(const vireo::Rect& scissors) { this->scissors = scissors; }

    protected:
        std::shared_ptr<Node> duplicateInstance() const override {
            return std::make_shared<Viewport>(viewport, scissors);
        }

    private:
        vireo::Viewport viewport{};
        vireo::Rect     scissors{};
    };

}
