/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
export module lysa.pipelines.draw_command_sort;

import vireo;
import lysa.frustum;
import lysa.global;
import lysa.memory;
import lysa.nodes.camera;

export namespace lysa {
    class DrawCommandSort {
    public:
        DrawCommandSort();

        void dispatch(
            vireo::CommandList& commandList,
            uint32 drawCommandsCount,
            const vireo::Buffer& input) ;

        virtual ~DrawCommandSort() = default;
        DrawCommandSort(DrawCommandSort&) = delete;
        DrawCommandSort& operator=(DrawCommandSort&) = delete;

    private:
        static constexpr vireo::DescriptorIndex BINDING_GLOBAL{0};
        static constexpr vireo::DescriptorIndex BINDING_INPUT{1};

        const std::wstring DEBUG_NAME{L"DrawCommandSort"};

        struct Global {
            uint32 drawCommandsCount;
        };

        std::shared_ptr<vireo::DescriptorLayout> descriptorLayout;
        std::shared_ptr<vireo::DescriptorSet>    descriptorSet;
        std::shared_ptr<vireo::Buffer>           globalBuffer;
        std::shared_ptr<vireo::Pipeline>         pipeline;
    };
}