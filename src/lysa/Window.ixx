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
     * Operating system window that serve as rendering surface.
     *  - Create and own the OS window/surface and its swap chain.
     *  - Drive the per‑frame loop (compute, pre‑render, render, present).
     *  - Host one or more Viewports that render Scenes into the swap chain.
     *  - Route input events to the UI system and active scene tree.
     *  - Provide convenience for mouse modes/cursor, post‑processing, and UI.
     *
     * Notes:
     *  - %A Window is typically created by Application from a WindowConfiguration.
     *  - Thread‑safety: unless stated otherwise, public methods should be called
     *    from the main/render thread. UI helpers are not thread‑safe.
     */
    class Window {
    public:
        /**
         * Constructs a Window and initializes the OS surface, swap chain, renderer
         * and optional root node.
         *
         * @param config    Window configuration (extent, title, rendering config, etc.).
         * @param rootNode  Optional scene root attached to the first viewport.
         */
        Window(WindowConfiguration& config, const std::shared_ptr<Node>& rootNode = nullptr);

        /** Called once the window is fully created and ready to display. */
        virtual void onReady() {}
        /** Called when the window is about to close (release resources here). */
        virtual void onClose() {}
        /** Called after the window/swap chain has been resized. */
        virtual void onResize() {}

        /**
         * Attaches a viewport to this window and wires it to the renderer.
         *
         * @param viewport Viewport to attach (must not be attached elsewhere).
         * @return The same viewport shared_ptr for chaining/convenience.
         */
        std::shared_ptr<Viewport> addViewport(const std::shared_ptr<Viewport>& viewport);

        /**
         * Returns the opaque, os-specific, window handle
         */
        // auto getWindowHandle() const { return windowHandle; }

        /** Returns the current aspect ratio (width/height) of the window. */
        auto getAspectRatio() const { return swapChain->getAspectRatio(); }

        /** Returns the current swap chain extent (width/height in pixels). */
        const auto& getExtent() const { return swapChain->getExtent(); }

        /** Returns the number of frames processed in flight. */
        const auto& getFramesInFlight() const { return config.renderingConfig.framesInFlight; }

        /** Returns the list of viewports attached to this window. */
        auto& getViewports() const { return viewports; }

        /** Returns the main scene renderer associated with this window. */
        auto& getRenderer() const { return *renderer; }

        /** Returns the UI renderer (Dear ImGui or equivalent wrapper). */
        auto& getUIRenderer() { return uiRenderer; }

        /** Blocks until the device/queues are idle for this window. */
        void waitIdle() const;

        /**
         * Adds a full‑screen post‑processing pass.
         *
         * @param fragShaderName Fragment shader identifier/name.
         * @param outputFormat   Output image format.
         * @param data           Optional extra parameters blob.
         * @param dataSize       Size in bytes of the extra parameters.
         */
        void addPostprocessing(
            const std::string& fragShaderName,
            vireo::ImageFormat outputFormat,
            void* data = nullptr, uint32 dataSize = 0) const;

        /** Removes a previously added post‑processing pass by fragment name. */
        void removePostprocessing(const std::string& fragShaderName) const;

        /** Makes the OS window visible. */
        void show() const;

        /** Requests the window to close and releases resources. */
        void close();

        /**
         * Sets the mouse visibility and capture mode
         *
         * @param mode MouseMode (visible, hidden, captured, etc.).
         */
        void setMouseMode(MouseMode mode) const;

        /**
         * Sets the mouse cursor
         *
         * @param cursor MouseCursor enum selecting the cursor shape.
         */
        void setMouseCursor(MouseCursor cursor) const;

        /**
         * Sets the mouse position to the center of the window
         */
        void resetMousePosition() const;

        /**
         * Returns the mouse position
         *
         * @return Mouse coordinates in pixels relative to the client area.
         */
        float2 getMousePosition() const;

        /**
         * Returns the mouse position
         *
         * @param position Coordinates in pixels relative to the client area.
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

        /** Returns the scaling factor used for the default font. */
        auto getFontScale() const { return windowManager.getDefaultFontScale(); }

        /** Returns the immutable window configuration used to create this window. */
        const auto& getConfiguration() const { return config; }

        /**
         * Updates graphics pipelines following a materials/pipelines mapping.
         * Typically called when Resources/Scene report material changes.
         *
         * @param pipelineIds Map of pipeline family id to materials.
         */
        void updatePipelines(const std::unordered_map<pipeline_id, std::vector<std::shared_ptr<Material>>>& pipelineIds) const;

        /** Destroys the window and releases owned resources. */
        virtual ~Window();
        Window(Window&) = delete;
        Window& operator=(Window&) = delete;

    private:
        /** Per‑frame resources and synchronization primitives. */
        struct FrameData {
            /** Fence signaled when the frame's work has completed on GPU. */
            std::shared_ptr<vireo::Fence> inFlightFence;
            /** Command allocator for this frame (resets between frames). */
            std::shared_ptr<vireo::CommandAllocator> commandAllocator;
            /** Command list used for compute workloads. */
            std::shared_ptr<vireo::CommandList> computeCommandList;
            /** Command list used for pre‑render tasks (uploads, layout transitions). */
            std::shared_ptr<vireo::CommandList> preRenderCommandList;
            /** Command list used for rendering into the swap chain. */
            std::shared_ptr<vireo::CommandList> renderCommandList;
            /** Semaphore signaled when compute stage is finished. */
            std::shared_ptr<vireo::Semaphore> computeSemaphore;
            /** Semaphore signaled when pre‑render stage is finished. */
            std::shared_ptr<vireo::Semaphore> preRenderSemaphore;
        };

        /** Window configuration (extent, title, rendering config). */
        WindowConfiguration&  config;
        /** Opaque OS window handle used for presentation. */
        void* windowHandle;
        /** True once the window has been requested to stop/close. */
        bool stopped{false};

        /** Array of per‑frame resource bundles (size = frames in flight). */
        std::vector<FrameData> framesData;
        /** Guards access to framesData when resized or recreated. */
        std::mutex frameDataMutex;
        /** Swap chain presenting to this window surface. */
        std::shared_ptr<vireo::SwapChain> swapChain{nullptr};
        /** Scene renderer used to draw attached viewports. */
        std::unique_ptr<Renderer> renderer;
        /** Viewports attached to this window. */
        std::vector<std::shared_ptr<Viewport>> viewports;
        /** Optional root node used by initial viewport. */
        std::shared_ptr<Node> rootNode;

        /** Immediate‑mode UI renderer/bridge. */
        UIRenderer uiRenderer;
        /** Window manager handling GUI windows/panels. */
        ui::WindowManager windowManager;

        /** Creates the underlying OS window and returns its native handle. */
        void* createWindow();

        friend class Application;
        friend class Input;

        /** Advances the window frame (update game/scene state). */
        void update();

        /** Records and submits GPU commands for the current frame. */
        void drawFrame();

        /** Steps the physics simulation for all attached viewports. */
        void physicsProcess(float delta) const;

        /** Calls process(alpha) on nodes for interpolation and per‑frame logic. */
        void process(float alpha) const;

        /** True when this is the primary application window. */
        bool mainWindow{false};

        /** Propagates input events to the UI manager and active scene tree. */
        void input(InputEvent &inputEvent);

        /** Recreates swap chain and dependent resources after a resize. */
        void resize();

        /** Signals readiness (calls onReady and performs post‑create setup). */
        void ready();

#ifdef _WIN32
        struct MonitorEnumData {
            int  enumIndex{0};
            int  monitorIndex{0};
            RECT monitorRect{};
        };
        /** Enum callback used to find monitors/rects for window placement. */
        static BOOL CALLBACK monitorEnumProc(HMONITOR, HDC, LPRECT, LPARAM);
        /** Win32 window procedure to handle OS messages/events. */
        static LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);

        RECT rect;
        /** Internal flag used to suppress synthetic mouse‑move feedback. */
        static bool resettingMousePosition;
        /** Cached OS cursors per MouseCursor enum value. */
        static std::map<MouseCursor, HCURSOR> mouseCursors;
#endif
    };

}