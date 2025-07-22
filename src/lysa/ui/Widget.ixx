/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.widget;

import lysa.enums;
import lysa.exception;
import lysa.object;
import lysa.types;
import lysa.resources.font;
import lysa.ui.rect;
import lysa.ui.resource;
import lysa.renderers.ui;

namespace lysa::ui {
    /**
     * Base class for all UI widgets
     */
    export class Widget : public Object {
    public:
        //! Widget type
        enum Type {
            //! transparent widget
            WIDGET,
            //! rectangular widget with only a background
            PANEL,
            //! rectangular widget with a border and a background
            BOX,
            //! %A horizontal or vertical line
            LINE,
            //! %A box with a title
            FRAME,
            //! %A push button
            BUTTON,
            //! %A two states button
            TOGGLEBUTTON,
            //! %A single line of text
            TEXT,
            //! An editable single line of text
            TEXTEDIT,
            //! %A scroll bar. with min, max & pos
            SCROLLBAR,
            //! Tree of Widget
            TREEVIEW,
            //! 2D Image
            IMAGE,
        };

        //! Widget placement (relative to the parent widget)
        enum AlignmentType {
            NONE,
            //! The child widget is centered and resized to the parent content size
            FILL,
            //! The child widget is centered (and take all the parent content size)
            CENTER,
            //! The child widget is horizontally centered
            HCENTER,
            //! The child widget is vertically centered
            VCENTER,
            //! The children are stack on the top
            TOP,
            //! The children are stack on the bottom
            BOTTOM,
            //! The children are stack on the left
            LEFT,
            //! The children are stack on the right
            RIGHT,
            //! The children are stack on the top and horizontally centered
            TOPCENTER,
            //! The children are stack on the bottom and horizontally centered
            BOTTOMCENTER,
            //! The children are stack on the left and vertically centered
            LEFTCENTER,
            //! The children are stack on the right and vertically centered
            RIGHTCENTER,
            //! The children are stack on the top and left aligned
            TOPLEFT,
            //! The children are stack on the bottom and left aligned
            BOTTOMLEFT,
            //! The children are stack on the bottom and right aligned
            BOTTOMRIGHT,
            //! The children are stack on the top and right aligned
            TOPRIGHT,
            //! The children are stack on the left then on the top
            LEFTTOP,
            //! The children are stack on the left then on the bottom
            LEFTBOTTOM,
            //! The children are stack on the right then on the bottom
            RIGHTBOTTOM,
            //! The children are stack on the right then on the top
            RIGHTTOP,
            //!
            CORNERTOPLEFT,
            //!
            CORNERTOPRIGHT,
            //!
            CORNERBOTTOMLEFT,
            //!
            CORNERBOTTOMRIGHT
        };

        /** Creates a widget of a particular type */
        Widget(Type = WIDGET);

        ~Widget() override = default;

        /** Returns the type of the widget */
        Type getType() const;

        /** Returns true if the widget is visible */
        bool isVisible() const;

        /** Shows or hides the widget */
        void show(bool show = true);

        /** Returns true is the widget is reactive to user action (mouse & keyboard) */
        bool isEnabled() const;

        /** Enables or disables widget reactions to input events */
        void enable(bool isEnabled = true);

        /** Moves the widget to a particular position. */
        void setPos(float x, float y);

        /** Returns the width of the widget, in pixels */
        float getWidth() const { return rect.width; }

        /** Returns the height of the widget, in pixels */
        float getHeight() const { return rect.height; }

        /** Resizes the widget */
        virtual void setSize(float width, float height);

        /** Returns the size & the position of the widget */
        const Rect &getRect() const;

        /** Changes the size & position of the widget */
        void setRect(float x, float y, float width, float height);

        /** Changes the size & position of the widget */
        void setRect(const Rect &rect);

        /** Returns the current widget placement */
        AlignmentType getAlignment() const;

        /** Sets the widget placement. Calling this method involve
            redrawing the parent widget & resizing all the children widgets */
        void setAlignment(AlignmentType alignment);

        /** Returns the current font of the widget */
        Font& getFont();

        /** Sets the current font of the widget */
        void setFont(const std::shared_ptr<Font>& font);

        /** Returns true if the widget has keyboard focus */
        bool isFocused() const;

        /** Returns the parent widget, or nullptr */
        Widget* getParent() const { return parent; }

        /** Adds a child widget.
              Children widgets will be destroyed on parent destruction.
                \param child	: child widget to add
                \param alignment: placement
                \param resource	: resource string
                \param overlap : overlap widget on top of other widgets

        */
        template<typename T>
        std::shared_ptr<T> add(
            std::shared_ptr<T> child,
            const AlignmentType alignment,
            const std::string &resource = "",
            const bool overlap = false) {
            assert([&]{return window != nullptr;}, "Widget must be added to a Window before adding child");
            if (!allowChildren) { return child; }
            children.push_back(child);
            _init(*child, alignment, resource, overlap);
            return child;
        }

        /** Removes a child widget */
        virtual void remove(const std::shared_ptr<Widget> &child);

        /** Removes all children's widgets recursively */
        virtual void removeAll();

        /** Changes children padding (space between children) */
        void setPadding(float padding);

        /** Returns current children padding (space between children) */
        float getPadding() const;

        float getVBorder() const;

        float getHBorder() const;

        void setVBorder(float size);

        void setHBorder(float size);

        /** Returns false if the background is transparent */
        bool isDrawBackground() const;

        /** Sets to false make the widget background transparent */
        void setDrawBackground(bool drawBackground);

        bool isPushed() const;

        bool isPointed() const;

        bool isFreezed() const;

        bool isRedrawOnMouseEvent() const;

        bool isOverlapping() const { return overlap; }

        Rect getChildrenRect() const;

        void setFreezed(const bool f) { freeze = f; }

        void setPushed(const bool p) { pushed = p; }

        /** Force a refresh of the entire widget */
        void refresh() const;

        /** Changes widget resources. Use with caution ! */
        void setResource(std::shared_ptr<Resource>);

        /** Return the user defined group index */
        uint32 getGroupIndex() const;

        /** Set the user defined group index */
        void setGroupIndex(int32 index);

        /** Returns the user data */
        void* getUserData() const;

        /** set user data */
        void setUserData(void *data);

        /** Return the transparency alpha value */
        float getTransparency() const { return transparency; }

        /** Changes the transparency alpha value */
        void setTransparency(float alpha);

        void resizeChildren();

        void _setRedrawOnMouseEvent(const bool r) { redrawOnMouseEvent = r; }

        void _setMoveChildrenOnPush(const bool r) { moveChildrenOnPush = r; }

        virtual std::list<std::shared_ptr<Widget>>& _getChildren() { return children; }

        void _draw(UIRenderer &) const;
        Widget* setFocus(bool = true);

        virtual void eventCreate();

        virtual void eventDestroy();

        virtual void eventShow();

        virtual void eventHide();

        virtual void eventEnable();

        virtual void eventDisable();

        virtual void eventMove(float x, float y);

        virtual void eventResize();

        virtual bool eventKeyDown(Key key);

        virtual bool eventKeyUp(Key key);

        virtual bool eventMouseDown(MouseButton button, float x, float y);

        virtual bool eventMouseUp(MouseButton button, float x, float y);

        virtual bool eventMouseMove(uint32, float x, float y);

        virtual void eventGotFocus();

        virtual void eventLostFocus();

        virtual void _setSize(float width, float height);

    protected:
        friend class Window;
        const Type type;
        Rect rect;
        Rect defaultRect;
        float hborder{0};
        float vborder{0};
        float padding{0};
        bool overlap{false};
        bool focused{false};
        bool allowFocus{false};
        bool allowChildren{true};
        bool drawBackground{true};
        bool moveChildrenOnPush{false};
        bool redrawOnMouseEvent{false};
        bool redrawOnMouseMove{false};
        float transparency{1.0f};
        Widget* parent{nullptr};
        AlignmentType alignment{NONE};
        std::shared_ptr<Resource> resource;
        std::list<std::shared_ptr<Widget>> children;
        Window* window{nullptr};
        void* style{nullptr};
        bool mouseMoveOnFocus{false};

        void allowingFocus(bool allow = true);

        virtual Rect _getDefaultRect() { return defaultRect; }

        virtual void _init(Widget &, AlignmentType, const std::string &, bool);

    private:
        bool pushed{false};
        bool pointed{false};
        bool freeze{true};
        bool enabled{true};
        bool visible{true};
        std::shared_ptr<Font> font{nullptr};
        void *userData{nullptr};
        int32 groupIndex{0};
        Rect childrenRect;

        Widget *setNextFocus();
    };
}