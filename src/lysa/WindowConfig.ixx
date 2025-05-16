/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.window_config;

import vireo;
import lysa.global;

export namespace lysa {

    struct MemoryConfig {
        uint32 maxModelsCount{100000};
        uint32 maxMaterialsCount{10000};
    };

    struct RenderingConfig {
        //! Graphic API
        vireo::Backend     backend{vireo::Backend::VULKAN};
        //! Swap chain image format
        vireo::ImageFormat renderingFormat{vireo::ImageFormat::R8G8B8A8_UNORM};
        //! MSAA samples count
        vireo::MSAA        msaa{vireo::MSAA::NONE};
        //! Presentation mode
        vireo::PresentMode presentMode = {vireo::PresentMode::IMMEDIATE};
        //! Frame buffer clear color
        float3             clearColor{DEFAULT_CLEAR_COLOR};
        //! Number of simultaneous frames during rendering
        uint32             framesInFlight{2};
        //! VRAM buffers max sizes
        MemoryConfig       memoryConfig;
    };

    /**
     * Rendering window configuration
     */
    struct WindowConfig {
        friend class Node;
        //! Startup Scene
        std::shared_ptr<Node> rootNode;
        //! Renderers configuration
        RenderingConfig       renderingConfig;
        //! Number of nodes updates per frame for asynchronous scene updates
        uint32                maxAsyncNodesUpdatedPerFrame{20};
    };
}
