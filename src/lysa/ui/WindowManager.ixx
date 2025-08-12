/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.ui.window_manager;

import lysa.constants;
import lysa.enums;
import lysa.input_event;
import lysa.object;
import lysa.types;
import lysa.renderers.ui;
import lysa.resources.font;
import lysa.ui.window;

export namespace lysa {

    class Window;

    namespace ui {
        /**
         * Manage all the UI windows
         */
        class WindowManager: public Object {
        public:
            /**
             * Adds a UI Window to the list of managed windows
             */
            std::shared_ptr<Window> add(const std::shared_ptr<Window>& window);

            /**
             * Removes a UI Window to the list of managed windows. The Window will be removed at the start of the next frame.
             */
            void remove(const std::shared_ptr<Window>& window);

            /**
             * Returns the default font loaded at startup
             */
            auto& getFont() const { return *defaultFont; }

            auto getFontScale() const { return fontScale; }

            /**
             * Forces a redrawing of all the UI at the start of the next frame
             */
            void refresh() { needRedraw = true; }

            UIRenderer& getRenderer() { return uiRenderer; }

            float getResizeDelta() const { return resizeDelta; }

            void setEnableWindowResizing(const bool enable) { enableWindowResizing = enable; }

            void drawFrame();

            bool onInput(InputEvent& inputEvent);

        private:
            const float resizeDelta{5.0f};
            UIRenderer& uiRenderer;
            lysa::Window& renderingWindow;
            std::shared_ptr<Font> defaultFont;
            std::list<std::shared_ptr<Window>> windows;
            std::mutex windowsMutex;
            std::vector<std::shared_ptr<Window>> removedWindows{};
            std::shared_ptr<Window> focusedWindow{nullptr};
            std::shared_ptr<Window> resizedWindow{nullptr};
            bool needRedraw{false};
            bool enableWindowResizing{true};
            bool resizingWindow{false};
            bool resizingWindowOriginBorder{false};
            MouseCursor currentCursor{MouseCursor::ARROW};
            float fontScale;

        public:
            WindowManager(lysa::Window& renderingWindow, UIRenderer&renderer, const std::string& defaultFontName, float defaultFontScale);
            ~WindowManager() override;
        };
    }

}