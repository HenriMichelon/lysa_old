/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.resource;

import std;
import lysa.object;

export namespace lysa::ui {

    /**
     * Super class for style resources descriptions
     */
    class Resource: public Object {
    public:
        Resource(const std::string& res): res{std::move(res)} {}

        ~Resource() override = default;

        const std::string& getResource() const { return res; }

    private:
        std::string res{};
    };

}