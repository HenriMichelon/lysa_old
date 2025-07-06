/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.configuration;

import vireo;
import lysa.global;
import lysa.physics.configuration;

export namespace lysa {

    //! Coloring scheme of collision shapes (only supported by Jolt)
    enum class DebugShapeColor {
        //! Random color per instance
        InstanceColor,
        //! Convex = green, scaled = yellow, compound = orange, mesh = red
        ShapeTypeColor,
        //! Static = grey, keyframed = green, dynamic = random color per instance
        MotionTypeColor,
        //! Static = grey, keyframed = green, dynamic = yellow, sleeping = red
        SleepColor,
    };

    /**
     * Configuration of the in-game debug
     */
    struct DebugConfig {
        //! Enable the debug visualization
        bool                enabled{false};
        //! If the debug renderer is enabled, display the debug at startup
        bool                displayAtStartup{true};
        //! Draw with depth-testing
        bool                depthTestEnable{true};
        //! Draw coordinate system (x = red, y = green, z = blue)
        bool                drawCoordinateSystem{false};
        //! Coordinate system draw scale
        float               coordinateSystemScale{1.0f};
        //! Draw all the rays of the RayCast objects
        bool                drawRayCast{false};
        //! Color for the non-colliding rays
        float4              rayCastColor{0.0f, 0.5f, 1.0f, 1.0f};
        //! Color for the colliding rays
        float4              rayCastCollidingColor{0.95f, 0.275f, 0.76f, 1.0f};
        //! Draw the collision shapes of all collision objects
		bool			    drawShape{true};
        //! Coloring scheme to use for collision shapes
		DebugShapeColor	    shapeColor{DebugShapeColor::ShapeTypeColor};
        //! Draw a bounding box per collision object
		bool				drawBoundingBox{false};
        //! Draw the velocity vectors
		bool				drawVelocity{false};
        //! Draw the center of mass for each collision object
        bool                drawCenterOfMass{false};
	};

    struct ResourcesConfiguration {
        uint32 maxVertexInstances{5000000};
        uint32 maxMaterialInstances{1000};
        uint32 maxIndexInstances{5000000*2};
        uint32 maxMeshSurfaceInstances{200000};
    };

    struct ApplicationConfiguration {
        //! Directory to search for resources for the app:// URI
        std::filesystem::path  appDir{L"."};
        //! Directory to search for compiled shaders inside app://
        std::wstring           shaderDir{L"shaders"};
        PhysicsConfiguration   physicsConfig{};
        //! Where to log a message using Logger
        int                    loggingMode{LOGGING_MODE_NONE};
        //! Minimum level for the log messages
        LogLevel               logLevelMin{LogLevel::INFO};
        //! Graphic API
        vireo::Backend         backend{vireo::Backend::VULKAN};
        //! Global resources configuration
        ResourcesConfiguration resourcesConfig;
    };

    struct SceneConfiguration {
        //! Number of nodes updates per frame for asynchronous scene updates
        uint32 maxAsyncNodesUpdatedPerFrame{20};
        uint32 maxModelsPerScene{10000};
        uint32 maxMeshSurfacePerPipeline{100000};
    };

    struct RenderingConfiguration {
        RendererType       rendererType{RendererType::DEFERRED};
        //! Main color rendering pass frame buffer format
        vireo::ImageFormat colorRenderingFormat{vireo::ImageFormat::R16G16B16A16_UNORM};
        //! Postprocessing & swap chain image format
        vireo::ImageFormat swapChainFormat{vireo::ImageFormat::R8G8B8A8_UNORM};
        //! Depth and stencil buffer format
        vireo::ImageFormat depthStencilFormat{vireo::ImageFormat::D32_SFLOAT_S8_UINT};
        //! MSAA samples count
        vireo::MSAA        msaa{vireo::MSAA::NONE};
        //! Presentation mode
        vireo::PresentMode presentMode{vireo::PresentMode::IMMEDIATE};
        //! Frame buffer clear color
        float3             clearColor{DEFAULT_CLEAR_COLOR};
        //! Number of simultaneous frames during rendering
        uint32             framesInFlight{2};
        //! Enable the bloom post-processing effect
        bool               bloomEnabled{true};
        //! Bloom effect blur kernel size
        uint32             bloomBlurKernelSize{5};
        //! Bloom effect blur strength
        float              bloomBlurStrength{1.2f};
        //! Enable SSAO in the deferred renderer
        bool               ssaoEnabled{true};
        //! SSAO blur kernel size
        uint32             ssaoBlurKernelSize{5};
        //! SSAO sampling count
        uint32             ssaoSampleCount{16};
        //! SSAO sampling radius
        float              ssaoRadius{0.5f};
        //! SSAO self-shadowing bias
        float              ssaoBias{0.025f};
        //! SSAO strength
        float              ssaoStrength{1.2f};
    };

    /**
     */
    struct ViewportConfiguration {
        friend class Node;
        vireo::Viewport    viewport{};
        vireo::Rect        scissors{};
        //! Scene resources configuration
        SceneConfiguration sceneConfig{};
        //! Debug configuration for this viewport
        DebugConfig        debugConfig{};
    };

    /**
     * Rendering window configuration
     */
    struct WindowConfiguration {
        //! Window title bar
        std::wstring            title{};
        //! State of the display Window
        WindowMode              mode{WindowMode::WINDOWED};
        //! Start up X position (top-left corner)
        int32                   x{-1};
        //! Start up Y position (top-left corner)
        int32                   y{-1};
        //! Width in pixels of the display Window
        uint32                  width{1280};
        //! Height in pixels of the display Window
        uint32                  height{720};
        //! Monitor index to display the Window
        int32                   monitor{0};
        //! Configuration of the main viewport
        ViewportConfiguration   mainViewportConfig{};
        //! Configuration of the various renderers
        RenderingConfiguration  renderingConfig{};
    };

}
