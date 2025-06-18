/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#ifdef JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.h>
#else
// Hack to still compile DebugRenderer when Jolt is compiled without
#define JPH_DEBUG_RENDERER
// Make sure the debug renderer symbols don't get imported or exported
#define JPH_DEBUG_RENDERER_EXPORT
#include <Jolt/Renderer/DebugRenderer.h>
#undef JPH_DEBUG_RENDERER
#undef JPH_DEBUG_RENDERER_EXPORT
#endif
#include <Jolt/Renderer/DebugRendererSimple.h>
#endif
#include <cstdlib>
export module lysa.renderers.debug;

import std;
import vireo;
import lysa.global;
import lysa.configuration;
import lysa.scene;
import lysa.nodes.node;
import lysa.renderers.vector;

export namespace lysa {

    class DebugRenderer : public VectorRenderer
#ifdef PHYSIC_ENGINE_JOLT
        , public JPH::DebugRendererSimple
#endif
    {
    public:
        DebugRenderer(
            const DebugConfig& config,
            const RenderingConfiguration& renderingConfiguration);

        void drawRayCasts(const std::shared_ptr<Node>& scene, const float4& rayColor, const float4& collidingRayColor);

        const auto& getConfiguration() const { return config; }

#ifdef PHYSIC_ENGINE_JOLT
        void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) override;

        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override {}
#endif

    private:
        const DebugConfig& config;
    };
}