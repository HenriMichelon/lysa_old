/*
 * Copyright (c) 2025-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.surface_config;

import vireo;
import lysa.global;

export namespace lysa {


    /**
     * Rendering surface configuration
     */
    struct SurfaceConfig {
        friend class Node;
        //! Startup Scene
        std::shared_ptr<Node>   rootNode;
        //! Graphic API
        vireo::Backend          backend{vireo::Backend::VULKAN};
        //! Swap chain image format
        vireo::ImageFormat      renderingFormat{vireo::ImageFormat::R8G8B8A8_UNORM};
        //! MSAA samples count
        vireo::MSAA             msaa{vireo::MSAA::NONE};
        //! Presentation mode
        vireo::PresentMode      presentMode = {vireo::PresentMode::IMMEDIATE};
        //! Frame buffer clear color
        float3                  clearColor{DEFAULT_CLEAR_COLOR};
        //! Number of simultaneous frames during rendering
        uint32_t                framesInFlight{2};
        //! Name for the default vertex shader for the scene renderer
        // std::string          sceneVertexShader{"default"};
        //! Name for the default fragment shader for the scene renderer
        // std::string          sceneFragmentShader{"default"};
    };
}
