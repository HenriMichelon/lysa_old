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
        uint32 maxVertexInstances{1000000};
        uint32 maxStagingVertexInstances{500000};
        uint32 maxIndexInstances{1000000};
        uint32 maxStagingIndexInstances{500000};
        uint32 maxMaterialInstances{1000};
        uint32 maxStagingMaterialInstances{500};
    };

    struct ApplicationConfiguration {
        //! Graphic API
        vireo::Backend         backend{vireo::Backend::VULKAN};
        //! xxx
        ResourcesConfiguration resourcesConfig;
    };

    struct SceneConfiguration {
        //! Number of nodes updates per frame for asynchronous scene updates
        uint32 maxAsyncNodesUpdatedPerFrame{20};
        uint32 maxMeshSurfacePerFrame{100000};
        uint32 maxVertexPerFrame{1000000};
    };

    struct RenderingConfiguration {
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
    };

    /**
     * Rendering window configuration
     */
    struct WindowConfiguration {
        friend class Node;
        //! Startup Scene
        std::shared_ptr<Node>  rootNode;
        //! Renderers configuration
        RenderingConfiguration renderingConfig;
        //! Scene configuration
        SceneConfiguration     sceneConfig;
    };

}
