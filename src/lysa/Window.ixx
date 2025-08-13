/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#include <windows.h>
#endif
export module lysa.window;

import vireo;
import lysa.configuration;
import lysa.enums;
import lysa.input_event;
import lysa.math;
import lysa.memory;
import lysa.types;
import lysa.viewport;
import lysa.nodes.node;
import lysa.resources.material;
import lysa.renderers.renderer;
import lysa.renderers.ui;
import lysa.renderers.vector;
import lysa.ui.window;
import lysa.ui.window_manager;

export namespace lysa {
    /**
     * %A Rendering surface
     */
    class Window {
    public:
        Window(WindowConfiguration& config, const std::shared_ptr<Node>& rootNode = nullptr);

        virtual void onReady() {}
        virtual void onClose() {}
        virtual void onResize() {}

        std::shared_ptr<Viewport> addViewport(const std::shared_ptr<Viewport>& viewport);

        /**
         * Returns the opaque, os-specific, window handle
         */
        // auto getWindowHandle() const { return windowHandle; }

        auto getAspectRatio() const { return swapChain->getAspectRatio(); }

        const auto& getExtent() const { return swapChain->getExtent(); }

        const auto& getFramesInFlight() const { return config.renderingConfig.framesInFlight; }

        auto& getViewports() const { return viewports; }

        auto& getRenderer() const { return *renderer; }

        auto& getUIRenderer() { return uiRenderer; }

        void waitIdle() const;

        void addPostprocessing(
            const std::string& fragShaderName,
            vireo::ImageFormat outputFormat,
            void* data = nullptr, uint32 dataSize = 0) const;

        void removePostprocessing(const std::string& fragShaderName) const;

        void show() const;

        void close();

        /**
         * Sets the mouse visibility and capture mode
         */
        void setMouseMode(MouseMode mode) const;

        /**
         * Sets the mouse cursor
         */
        void setMouseCursor(MouseCursor cursor) const;

        /**
         * Sets the mouse position to the center of the window
         */
        void resetMousePosition() const;

        /**
         * Returns the mouse position
         */
        float2 getMousePosition() const;

        /**
         * Returns the mouse position
         */
        void setMousePosition(const float2& position) const;

        /**
       * Adds a GUI Window to the Window manager for display
       * @param window    The window to display must not be yet added to the window manager
       */
        std::shared_ptr<ui::Window> add(const std::shared_ptr<ui::Window> &window);

        /**
         * Removes the Window from the Window manager
         * @param window    The window to remove must be added to the window manager before
         */
        void remove(const std::shared_ptr<ui::Window> &window);

        /**
          * Returns the default font loaded at startup
          */
        auto& getFont() const { return windowManager.getDefaultFont(); }

        auto getFontScale() const { return windowManager.getDefaultFontScale(); }

        const auto& getConfiguration() const { return config; }

        void updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) const;

        virtual ~Window();
        Window(Window&) = delete;
        Window& operator=(Window&) = delete;

    private:
        // Per frame data
        struct FrameData {
            // frames rendering & presenting synchronization
            std::shared_ptr<vireo::Fence> inFlightFence;
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            std::shared_ptr<vireo::CommandList> computeCommandList;
            std::shared_ptr<vireo::CommandList> preRenderCommandList;
            std::shared_ptr<vireo::CommandList> renderCommandList;
            std::shared_ptr<vireo::Semaphore> computeSemaphore;
            std::shared_ptr<vireo::Semaphore> preRenderSemaphore;
        };

        WindowConfiguration&  config;
        // Opaque window handle for presenting
        void* windowHandle;
        bool stopped{false};

        // Per frame data
        std::vector<FrameData> framesData;
        std::mutex frameDataMutex;
        // Swap chain for this surface
        std::shared_ptr<vireo::SwapChain> swapChain{nullptr};
        // Scene renderer
        std::unique_ptr<Renderer> renderer;
        std::vector<std::shared_ptr<Viewport>> viewports;
        std::shared_ptr<Node> rootNode;

        UIRenderer uiRenderer;
        ui::WindowManager windowManager;

        void* createWindow();

        friend class Application;
        friend class Input;

        void update();

        void drawFrame();

        void physicsProcess(float delta) const;

        void process(float alpha) const;

        bool mainWindow{false};

        // Propagate the input event to the UI Window manager and to the current scene tree
        void input(InputEvent &inputEvent);

        void resize();

        void ready();

#ifdef _WIN32
        struct MonitorEnumData {
            int  enumIndex{0};
            int  monitorIndex{0};
            RECT monitorRect{};
        };
        static BOOL CALLBACK monitorEnumProc(HMONITOR, HDC, LPRECT, LPARAM);
        static LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);

        RECT rect;
        static bool resettingMousePosition;
        static std::map<MouseCursor, HCURSOR> mouseCursors;
#endif
    };

}