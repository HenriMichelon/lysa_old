/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.style;

import std;
import lysa.object;
import lysa.renderers.ui;
import lysa.resources.font;
import lysa.ui.rect;
import lysa.ui.resource;
import lysa.ui.widget;

namespace lysa::ui {

    /**
     * UI Widget drawing style base class
     */
    export class Style : public Object {
    public:
        Style();

        ~Style() override = default;

        /**
         * Creates a new UI drawing style renderer.
         *     @param name	: style name
        */
        static std::shared_ptr<Style> create(const std::string & name= "vector");

        /**
         * Create a resource from a resources description string.
         * @param widget : widget to ass resources string to
         * @param resources : string that describe the resources of a widget
        */
        virtual void addResource(Widget &widget, const std::string &resources) = 0;

        /**
         * Sets a style-specific option
        */
        virtual void setOption(const std::string &name, const std::string &value);

        /**
         * Returns a style-specific option value
        */
        std::string getOption(const std::string &name) const;

        /* Draws a widget.
         * @param widget : widget to draw
         * @param resources : resources used for drawing this widget
         * @param when : `true` = before drawing children, `false` = after
        */
        virtual void draw(const Widget &widget, Resource &resources, UIRenderer &render, bool when) const = 0;

        /**
         * Adjusts a widget size to style specific constraints
        */
        virtual void resize(Widget &widget, Rect &rect, Resource &resources) = 0;

        /**
         * Returns the default font for the style.
         */
        std::shared_ptr<Font> getFont() const { return font; }

    protected:
        std::shared_ptr<Font> font;

        virtual void init() {}

        virtual void updateOptions() = 0;

    private:
        class StyleOption {
        public:
            std::string name;
            std::string value;
            explicit StyleOption(const std::string &name):
                name{std::move(name)} {
            }
        };

        std::list<std::shared_ptr<StyleOption>> options;
    };
}
