/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.surface;

import std;
import vireo;
import lysa.surface_config;

export namespace lysa {

    /*
     * %A Rendering surface
     */
    class Surface {
    public:
        //! Creates a rendering surface
        Surface(SurfaceConfig& surfaceConfig, void* windowHandle);

        //! Prepare and draw a frame
        void drawFrame();

        auto getWindowHandle() const { return windowHandle; }

        auto getVireo() const { return vireo; }

        auto getConfig() const { return surfaceConfig; }

        virtual ~Surface() = default;
    private:
        void* windowHandle;
        SurfaceConfig& surfaceConfig;
        std::shared_ptr<vireo::Vireo> vireo;

        // Fixed delta time for the physics
        static constexpr float dt{1.0f/60.0f};
        // Last drawFrame() start time
        double currentTime{0.0};
        // Time accumulator to calculate the process delta time
        double accumulator{0.0};
        // Number of frames in the last second
        uint32_t frameCount{0};
        // Number of seconds since the last FPS update
        float elapsedSeconds{0.0f};
        // Average FPS,
        uint32_t fps{0};


    };

}