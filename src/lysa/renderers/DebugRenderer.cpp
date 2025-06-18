/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module;
#ifdef PHYSIC_ENGINE_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Renderer/DebugRenderer.h>
#ifndef JPH_DEBUG_RENDERER
// Hack to still compile DebugRenderer when Jolt is compiled without
#define JPH_DEBUG_RENDERER
#include <Jolt/Renderer/DebugRenderer.cpp>
#undef JPH_DEBUG_RENDERER
#endif // !JPH_DEBUG_RENDERER
#endif // PHYSIC_ENGINE_JOLT
module lysa.renderers.debug;

import lysa.application;
import lysa.virtual_fs;
import lysa.nodes.ray_cast;

namespace lysa {

    DebugRenderer::DebugRenderer(
        const DebugConfig& config,
        const RenderingConfiguration& renderingConfiguration,
        const std::wstring& name) :
        VectorRenderer{config.depthTestEnable, renderingConfiguration, name},
        config{config}{
    }

#ifdef PHYSIC_ENGINE_JOLT
    void DebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, const JPH::ColorArg inColor) {
        const auto color = float4{inColor.r, inColor.g, inColor.b, inColor.a} / 255.0f;
        linesVertices.push_back( {{ inFrom.GetX(), inFrom.GetY(), inFrom.GetZ() }, color });
        linesVertices.push_back( {{ inTo.GetX(), inTo.GetY(), inTo.GetZ() }, color});
        vertexBufferDirty = true;
    }

    void DebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, JPH::DebugRenderer::ECastShadow inCastShadow) {
        const auto color = float4{inColor.r, inColor.g, inColor.b, inColor.a} / 255.0f;
        triangleVertices.push_back( {{ inV1.GetX(), inV1.GetY(), inV1.GetZ() }, color });
        triangleVertices.push_back( {{ inV2.GetX(), inV2.GetY(), inV2.GetZ() }, color});
        triangleVertices.push_back( {{ inV3.GetX(), inV3.GetY(), inV3.GetZ() }, color});
        vertexBufferDirty = true;
    }
#endif

    void DebugRenderer::drawRayCasts(const std::shared_ptr<Node>& scene, const float4& rayColor, const float4& collidingRayColor) {
        for(const auto& rayCast : scene->findAllChildren<RayCast>(true)) {
            drawLine(
                rayCast->getPositionGlobal(),
                rayCast->isColliding() ? rayCast->getCollisionPoint() : rayCast->toGlobal(rayCast->getTarget()),
                rayCast->isColliding() ? collidingRayColor : rayColor);
        }
    }

}