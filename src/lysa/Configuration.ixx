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

    struct MemoryConfiguration {
        uint32 maxMeshSurfacePerBufferCount{10000};
        uint32 maxMaterialCount{1000};
    };

    struct RenderingConfiguration {
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
        MemoryConfiguration       memoryConfig;
    };

    /**
     * Rendering window configuration
     */
    struct WindowConfiguration {
        friend class Node;
        //! Startup Scene
        std::shared_ptr<Node> rootNode;
        //! Renderers configuration
        RenderingConfiguration       renderingConfig;
        //! Number of nodes updates per frame for asynchronous scene updates
        uint32                maxAsyncNodesUpdatedPerFrame{20};
    };
}
