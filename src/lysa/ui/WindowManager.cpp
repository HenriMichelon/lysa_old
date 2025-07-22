/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#include <windows.h>
#endif
module lysa.ui.window_manager;

import lysa.enums;
import lysa.math;
import lysa.resources.font;
import lysa.input;
import lysa.input_event;
import lysa.types;
import lysa.ui.rect;
import lysa.ui.window;
import lysa.renderers.ui;

namespace lysa::ui {

    WindowManager::WindowManager(
        lysa::Window& renderingWindow,
        UIRenderer& renderer,
        const std::string& defaultFontName,
        const uint32 defaultFontSize):
        renderingWindow{renderingWindow},
        uiRenderer{renderer} {
        defaultFont = make_shared<Font>(defaultFontName, defaultFontSize);
    }

    WindowManager::~WindowManager() {
        for (const auto& window: windows) {
            window->eventDestroy();
        }
        windows.clear();
    }

    void WindowManager::drawFrame() {
        auto lock = std::lock_guard(windowsMutex);
        for(const auto&window : removedWindows) {
            window->detach();
            if (window->isVisible()) { window->eventHide(); }
            window->eventDestroy();
            windows.remove(window);
            needRedraw = true;
        }
        removedWindows.clear();
        for (auto& window: windows) {
            if (window->visibilityChanged) {
                window->visibilityChanged = false;
                window->visible = window->visibilityChange;
                needRedraw = true;
                if (window->visible) {
                    if (focusedWindow) { focusedWindow->eventLostFocus(); }
                    focusedWindow = window;
                    window->eventGotFocus();
                    window->eventShow();
                } else {
                    if (focusedWindow == window) {
                        window->eventLostFocus();
                        if (windows.empty()) {
                            focusedWindow = nullptr;
                        } else {
                            focusedWindow = windows.back();
                            focusedWindow->eventGotFocus();
                        }
                    }
                    window->eventHide();
                }
            }
        }
        if (!needRedraw) { return; }
        needRedraw = false;
        uiRenderer.restart();
        for (const auto& window: windows) {
            window->draw();
        }
    }

    std::shared_ptr<Window> WindowManager::add(const std::shared_ptr<Window> &window) {
        {
            auto lock = std::lock_guard(windowsMutex);
            windows.push_back(window);
            window->attach(this);
        }
        window->eventCreate();
        if (window->isVisible()) { window->eventShow(); }
        needRedraw = true;
        return window;
    }

    void WindowManager::remove(const std::shared_ptr<Window>&window) {
        removedWindows.push_back(window);
    }

    bool WindowManager::onInput(InputEvent &inputEvent) {
        if (inputEvent.getType() == InputEventType::KEY) {
            const auto &keyInputEvent = dynamic_cast<InputEventKey &>(inputEvent);
            if ((focusedWindow != nullptr) && (focusedWindow->isVisible())) {
                if (keyInputEvent.isPressed()) {
                    return focusedWindow->eventKeyDown(keyInputEvent.getKey());
                } else {
                    return focusedWindow->eventKeyUp(keyInputEvent.getKey());
                }
            }
        } else if ((inputEvent.getType() == InputEventType::MOUSE_BUTTON)
                || (inputEvent.getType() == InputEventType::MOUSE_MOTION)) {
#ifdef _WIN32
            CURSORINFO ci {
                .cbSize = sizeof(CURSORINFO)
            };
            if (GetCursorInfo(&ci)) {
                // Mouse cursor is hidden
                if (ci.flags == 0) {
                    return false;
                }
            }
#endif
            auto &mouseEvent = dynamic_cast<InputEventMouse&>(inputEvent);
            const auto scaleX = uiRenderer.getExtent().x / renderingWindow.getExtent().width;
            const auto scaleY = uiRenderer.getExtent().y / renderingWindow.getExtent().height;
            const auto x = mouseEvent.getX() * scaleX;
            const auto y = mouseEvent.getY() * scaleY;

            if (inputEvent.getType() == InputEventType::MOUSE_MOTION) {
                const auto resizeDeltaY = scaleY * resizeDelta;
                if (resizedWindow != nullptr) {
                    if (resizingWindow) {
                        Rect rect = resizedWindow->getRect();
                        if (currentCursor == MouseCursor::RESIZE_H) {
                            const auto lx = x - rect.x;
                            if (resizingWindowOriginBorder) {
                                rect.width = rect.width - lx;
                                rect.x = x;
                            } else {
                                rect.width = lx;
                            }
                        } else {
                            const auto ly = y - rect.y;
                            if (resizingWindowOriginBorder) {
                                rect.height = rect.height - ly;
                                rect.y = y;
                            } else {
                                rect.height = ly;
                            }
                        }
                        resizedWindow->setRect(rect);
                        renderingWindow.setMouseCursor(currentCursor);
                        return true;
                    }
                    currentCursor = MouseCursor::ARROW;
                    resizedWindow = nullptr;
                    renderingWindow.setMouseCursor(currentCursor);
                }
                for (const auto& window: windows) {
                    auto consumed = false;
                    const float lx = std::ceil(x - window->getRect().x);
                    const float ly = std::ceil(y - window->getRect().y);
                    if (window->getRect().contains(x, y)) {
                        if (enableWindowResizing && window->getWidget().isDrawBackground()) {
                            if ((window->getResizeableBorders() & Window::RESIZEABLE_RIGHT) &&
                                (lx >= (window->getRect().width - resizeDelta))) {
                                currentCursor = MouseCursor::RESIZE_H;
                                resizedWindow = window;
                                resizingWindowOriginBorder = false;
                                } else if ((window->getResizeableBorders() & Window::RESIZEABLE_LEFT) &&
                                           (lx < resizeDelta)) {
                                    currentCursor = MouseCursor::RESIZE_H;
                                    resizedWindow = window;
                                    resizingWindowOriginBorder = true;
                                           } else if ((window->getResizeableBorders() & Window::RESIZEABLE_TOP) &&
                                                      (ly >= static_cast<float>(window->getRect().height - resizeDeltaY))) {
                                               currentCursor = MouseCursor::RESIZE_V;
                                               resizedWindow = window;
                                               resizingWindowOriginBorder = false;
                                                      } else if ((window->getResizeableBorders() & Window::RESIZEABLE_BOTTOM) &&
                                                                 (ly < static_cast<float>(resizeDeltaY))) {
                                                          currentCursor = MouseCursor::RESIZE_V;
                                                          resizedWindow = window;
                                                          resizingWindowOriginBorder = true;
                                                                 }
                        }
                        if (resizedWindow != nullptr) {
                            renderingWindow.setMouseCursor(currentCursor);
                            return true;
                        }
                        consumed |= window->eventMouseMove(mouseEvent.getButtonsState(), lx, ly);
                    }
                    if (consumed) { return true; }
                }
            } else {
                const auto &mouseInputEvent = dynamic_cast<InputEventMouseButton&>(mouseEvent);
                if (resizedWindow != nullptr) {
                    if ((!resizingWindow) &&
                        (mouseInputEvent.getMouseButton() == MouseButton::LEFT) &&
                        (mouseInputEvent.isPressed())) {
                        resizingWindow = true;
                        } else if ((mouseInputEvent.getMouseButton() == MouseButton::LEFT) &&
                                   (!mouseInputEvent.isPressed())) {
                            currentCursor = MouseCursor::ARROW;
                            resizedWindow = nullptr;
                            resizingWindow = false;
                                   }
                    renderingWindow.setMouseCursor(currentCursor);
                    return true;
                }
                for (const auto& window: windows) {
                    auto consumed = false;
                    const auto lx = std::ceil(x - window->getRect().x);
                    const auto ly = std::ceil(y - window->getRect().y);
                    if (mouseInputEvent.isPressed()) {
                        if (window->getRect().contains(x, y)) {
                            consumed |= window->eventMouseDown(mouseInputEvent.getMouseButton(), lx, ly);
                        }
                    } else {
                        consumed |= window->eventMouseUp(mouseInputEvent.getMouseButton(), lx, ly);
                    }
                    if (consumed) { return true; }
                }
            }
                }
        return false;
    }

}