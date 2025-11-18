/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module lysa.ui.window;

import lysa.enums;
import lysa.exception;
import lysa.resources.font;
import lysa.ui.event;
import lysa.ui.panel;
import lysa.ui.rect;
import lysa.ui.style;
import lysa.ui.widget;
import lysa.ui.window_manager;
import lysa.renderers.ui;

namespace lysa::ui {

    Window::Window(const Rect& rect):
        rect{rect} {
    }

    void Window::attach(WindowManager* windowManager) {
        assert([&]{ return this->windowManager == nullptr;} , "ui::Window must not be already attached to a manager");
        this->windowManager = windowManager;
        this->fontScale = windowManager->getDefaultFontScale();
        this->font = windowManager->getDefaultFont();
        this->textColor = windowManager->getDefaultTextColor();
    }

    void Window::detach() {
        windowManager = nullptr;
    }

    void Window::draw() const {
        if (!isVisible()) { return; }
        UIRenderer& renderer = windowManager->getRenderer();
        renderer.setTranslate({rect.x, rect.y});
        renderer.setTransparency(1.0f - transparency);
        widget->_draw(renderer);
    }

    void Window::unFreeze(const std::shared_ptr<Widget> &widget) {
        for (auto &child : widget->_getChildren()) {
            unFreeze(child);
        }
        widget->setFreezed(false);
    }

    void Window::setFont(const std::shared_ptr<Font>& font) {
        this->font = font;
        refresh();
    }

    std::shared_ptr<Font> Window::getFont() const {
        return font;
    }

    float Window::getFontScale() const {
        return fontScale;
    }

    void Window::setFontScale(const float fontScale) {
        this->fontScale = fontScale;
        refresh();
    }

    void Window::setWidget(std::shared_ptr<Widget> child, const std::string &resources, const float padding) {
        assert([&]{ return windowManager != nullptr;} , "ui::Window must be added to a Window manager before setting the main widget");
        if (layout == nullptr) { setStyle(nullptr); }
        if (widget == nullptr) {
            widget = std::make_shared<Widget>();
        } else {
            widget = std::move(child);
        }
        widget->setFreezed( true);
        widget->setPadding(padding);
        widget->window = this;
        widget->style = layout.get();
        widget->setFont(static_cast<Style*>(widget->style)->getFont());
        static_cast<Style*>(widget->style)->addResource(*widget, resources);
        widget->eventCreate();
        widget->setPos(0, 0);
        widget->_setSize(getWidth(), getHeight());
        focusedWidget = widget->setFocus();
        unFreeze(widget);
    }

    void Window::setStyle(const std::shared_ptr<Style>& style) {
        if (layout == nullptr) {
            layout = Style::create();
        } else {
            layout = std::move(style);
        }
        refresh();
    }

    void Window::setVisible(const bool isVisible) {
        if (visible != isVisible) {
            visibilityChange  = isVisible;
            visibilityChanged = true;
        }
    }

    void Window::hide() {
        setVisible(false);
    }

    void Window::show() {
        setVisible(true);
    }

    void Window::eventCreate() {
        if (rect == FULLSCREEN) {
            rect.width = maxWidth;
            rect.height = maxHeight;
        }
        setWidget();
        onCreate();
        emit(Event::OnCreate);
        if (widget != nullptr) { widget->resizeChildren(); }
    }

    void Window::eventDestroy() {
        if (widget) { widget->eventDestroy(); }
        emit(Event::OnDestroy);
        onDestroy();
        widget.reset();
    }

    void Window::eventShow() {
        if (widget) { widget->eventShow(); }
        onShow();
        emit(Event::OnShow);
        refresh();
    }

    bool Window::eventKeyDown(const Key K) {
        bool consumed = false;
        if (focusedWidget) {
            consumed = focusedWidget->eventKeyDown(K); // XXX consumed
        }
        if (!consumed) {
            consumed |= onKeyDown(K);
        }
        if (!consumed) {
            auto event = EventKeyb{.key = K};
            emit(Event::OnKeyDown, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventKeyUp(const Key K) {
        bool consumed = false;
        if (focusedWidget) {
            focusedWidget->eventKeyUp(K); // XXX consumed
        }
        if (!consumed) {
            consumed |= onKeyUp(K);
        }
        if (!consumed) {
            auto event = EventKeyb{.key = K};
            emit(Event::OnKeyUp, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventMouseDown(const MouseButton B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) {
            consumed = widget->eventMouseDown(B, X, Y);
        }
        if (!consumed) {
            consumed |= onMouseDown(B, X, Y);
        }
        if (!consumed) {
            auto event = EventMouseButton{.button = B, .x = X, .y = Y};
            emit(Event::OnMouseDown, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventMouseUp(const MouseButton B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) { consumed = widget->eventMouseUp(B, X, Y); }
        if (!consumed) {
            consumed |= onMouseUp(B, X, Y);
        }
        if (!consumed) {
            auto event = EventMouseButton{.button = B, .x = X, .y = Y};
            emit(Event::OnMouseUp, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventMouseMove(const uint32 B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if ((focusedWidget != nullptr) &&
            (focusedWidget->mouseMoveOnFocus)) {
            consumed = focusedWidget->eventMouseMove(B, X, Y);
            } else if (widget) {
                consumed = widget->eventMouseMove(B, X, Y);
            }
        if (!consumed) {
            consumed |= onMouseMove(B, X, Y);
        }
        if (!consumed) {
            auto event = EventMouseMove{.buttonsState = B, .x = X, .y = Y};
            emit(Event::OnMouseMove, &event);
            consumed = event.consumed;
        }
        if (consumed) { refresh(); }
        return consumed;
    }

    void Window::refresh() const {
        if (windowManager) { windowManager->refresh(); }
    }

    void Window::setFocusedWidget(const std::shared_ptr<Widget> &W) {
        focusedWidget = W.get();
    }

    Widget &Window::getWidget() const {
        assert([&]{ return windowManager != nullptr;} , "ui::Window not attached to a manager");
        return *widget;
    }

    void Window::setRect(const Rect &newRect) {
        rect = newRect;
        rect.width = std::min(std::max(newRect.width, minWidth), maxWidth);
        rect.height = std::min(std::max(newRect.height, minHeight), maxHeight);
        eventResize();
    }

    void Window::setHeight(const float height) {
        rect.height = std::min(std::max(height, minHeight), maxHeight);
        eventResize();
    }

    void Window::setWidth(const float width) {
        rect.width = std::min(std::max(width, minWidth), maxWidth);
        eventResize();
    }

    void Window::setPos(const float x, const float y) {
        rect.x = x;
        rect.y = y;
        eventMove();
    }

    void Window::setPos(const float2& pos) {
        rect.x = pos.x;
        rect.y = pos.y;
        eventMove();
    }

    void Window::setX(const float x) {
        rect.x = x;
        eventMove();
    }

    void Window::setY(const float y) {
        rect.y = y;
        eventMove();
    }

    std::shared_ptr<Style> Window::getStyle() const {
        return layout;
    }

    void Window::setTransparency(const float alpha) {
        transparency = alpha;
        refresh();
    }

    void Window::eventResize() {
        if (widget) { widget->_setSize(rect.width, rect.height); }
        onResize();
        emit(Event::OnResize);
        refresh();
    }

    void Window::eventMove() {
        if (widget) { widget->resizeChildren(); }
        onMove();
        emit(Event::OnMove);
        refresh();
    }

    void Window::eventHide() {
        emit(Event::OnHide);
        onHide();
        refresh();
    }

    void Window::eventGotFocus() {
        onGotFocus();
        emit(Event::OnGotFocus);
        refresh();
    }

    void Window::eventLostFocus() {
        onLostFocus();
        emit(Event::OnLostFocus);
        refresh();
    }

    void Window::setMinimumSize(const float width, const float height) {
        minWidth  = width;
        minHeight = height;
    }

    void Window::setMaximumSize(const float width, const float height) {
        maxWidth  = width;
        maxHeight = height;
    }
}

