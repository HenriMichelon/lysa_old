/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.configuration;

import vireo;
import lysa.global;

export namespace lysa {

    struct ResourcesConfiguration {
        uint32 maxVertexInstances{5000000};
        uint32 maxIndexInstances{1000000};
        uint32 maxMaterialInstances{1000};
    };

    struct ApplicationConfiguration {
        //! Directory to search for resources and compiled shaders for the app:// URI
        std::filesystem::path  appDir{L"."};
        //! Where to log message using log()
        int                    loggingMode{LOGGING_MODE_NONE};
        //! Monitor index for the logging Window
        LogLevel               logLevelMin{LogLevel::INFO};
        //! Graphic API
        vireo::Backend         backend{vireo::Backend::VULKAN};
        ResourcesConfiguration resourcesConfig;
    };

    struct SceneConfiguration {
        //! Number of nodes updates per frame for asynchronous scene updates
        uint32 maxAsyncNodesUpdatedPerFrame{20};
        uint32 maxMeshSurfacePerFrame{100000};
        uint32 maxVertexPerFrame{10000000};
    };

    struct RenderingConfiguration {
        //! Swap chain image format
        vireo::ImageFormat renderingFormat{vireo::ImageFormat::R8G8B8A8_UNORM};
        //! Depth and stencil buffer format
        vireo::ImageFormat depthStencilFormat{vireo::ImageFormat::D32_SFLOAT};
        //! MSAA samples count
        vireo::MSAA        msaa{vireo::MSAA::NONE};
        //! Presentation mode
        vireo::PresentMode presentMode{vireo::PresentMode::IMMEDIATE};
        //! Frame buffer clear color
        float3             clearColor{DEFAULT_CLEAR_COLOR};
        //! Number of simultaneous frames during rendering
        uint32             framesInFlight{2};
        bool               forwardDepthPrepass{false};
    };

    /**
     */
    struct ViewportConfiguration {
        friend class Node;
        vireo::Viewport        viewport{};
        vireo::Rect            scissors{};
        SceneConfiguration     sceneConfig{};
    };

    /**
     * Rendering window configuration
     */
    struct WindowConfiguration {
        //! Window title bar
        std::wstring            title{};
        //! State of the display Window
        WindowMode              mode{WindowMode::WINDOWED};
        //! Startup X position (top-left corner)
        int32                   x{-1};
        //! Startup Y position (top-left corner)
        int32                   y{-1};
        //! Width in pixels of the display Window
        uint32                  width{1280};
        //! Height in pixels of the display Window
        uint32                  height{720};
        //! Monitor index to display the Window
        int32                   monitor{0};

        ViewportConfiguration   mainViewportConfig{};
        RenderingConfiguration  renderingConfig{};
    };

}
