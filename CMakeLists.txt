#
# Copyright (c) 2025-present Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
cmake_minimum_required(VERSION 3.29)

#######################################################
project(lysa)
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env.cmake")
    message(FATAL_ERROR "Please create a .env.cmake file with the VIREO_PROJECT_DIR variable")
endif ()
include(.env.cmake)
if (NOT DEFINED VIREO_PROJECT_DIR)
    message(FATAL_ERROR "Please set VIREO_PROJECT_DIR in the .env.cmake file")
endif()

#######################################################
set(LYSA_TARGET ${PROJECT_NAME})
set(DIRECTX_BACKEND ON)
set(PHYSIC_ENGINE_JOLT ON)
set(PHYSIC_ENGINE_PHYSX OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#######################################################
if(WIN32)
    if(MSVC)
        set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
    endif()
    add_compile_definitions(UNICODE _UNICODE)
endif ()

#######################################################
if(NOT PHYSIC_ENGINE_JOLT AND NOT PHYSIC_ENGINE_PHYSX)
    message(FATAL_ERROR "Please activate a physics library with PHYSIC_ENGINE_JOLT ou PHYSIC_ENGINE_PHYSX")
endif()
if(PHYSIC_ENGINE_JOLT AND PHYSIC_ENGINE_PHYSX)
    message(FATAL_ERROR "Please deactivate a physics library")
endif()
if(PHYSIC_ENGINE_JOLT)
    add_compile_definitions(PHYSIC_ENGINE_JOLT)
    add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR)
    #add_compile_definitions(JPH_USE_STD_VECTOR)
endif()
if (PHYSIC_ENGINE_PHYSX)
    if (NOT DEFINED PHYSX_ROOT)
        message(FATAL_ERROR "Please set PHYSX_ROOT in the .env.cmake file")
    endif()
    if (NOT DEFINED PHYSX_LIBRARIES)
        message(FATAL_ERROR "Please set PHYSX_LIBRARIES in the .env.cmake file")
    endif()
    add_compile_definitions(PHYSIC_ENGINE_PHYSX PX_PHYSX_STATIC_LIB PX_FOUNDATION_STATIC_LIB PX_PHYSX_COMMON_STATIC_LIB)
endif()

#######################################################
include(FetchContent)
include(cmake/libraries.cmake)
include(cmake/shaders.cmake)

#######################################################
add_subdirectory(${VIREO_PROJECT_DIR} external_lib_build)
set(VIREO_TARGET "vireo_rhi")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ENGINE_SRC_DIR ${SRC_DIR}/${LYSA_TARGET})
set(SHADERS_SRC_DIR ${ENGINE_SRC_DIR}/shaders)
set(SHADERS_INCLUDE_DIR ${ENGINE_SRC_DIR}/shaders/include)
set(SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(DEPENDS_SRC_DIR ${ENGINE_SRC_DIR}/depends)

set(HLSLPP_TARGET hlslpp)
set(HLSLPP_SRC_DIR ${DEPENDS_SRC_DIR}/${HLSLPP_TARGET})

#######################################################
# Slang shaders
file(MAKE_DIRECTORY ${SHADERS_BUILD_DIR})
set(SHADERS_SOURCE_FILES
#        "${SHADERS_SRC_DIR}/default.vert.slang"
#        "${SHADERS_SRC_DIR}/depth_prepass.vert.slang"
#        "${SHADERS_SRC_DIR}/frustum_culling.comp.slang"
#        "${SHADERS_SRC_DIR}/frustum_culling_shadowmap.comp.slang"
#        "${SHADERS_SRC_DIR}/quad.vert.slang"
        "${SHADERS_SRC_DIR}/vector.slang"
        "${SHADERS_SRC_DIR}/vector_ui.slang"
        "${SHADERS_SRC_DIR}/glyph.slang"
        "${SHADERS_SRC_DIR}/glyph_ui.slang"
#        "${SHADERS_SRC_DIR}/deferred/gbuffers.frag.slang"
#        "${SHADERS_SRC_DIR}/deferred/glighting.frag.slang"
#        "${SHADERS_SRC_DIR}/deferred/glighting_bloom.frag.slang"
#        "${SHADERS_SRC_DIR}/deferred/ssao.frag.slang"
#        "${SHADERS_SRC_DIR}/forward/forward.frag.slang"
#        "${SHADERS_SRC_DIR}/forward/forward_bloom.frag.slang"
#        "${SHADERS_SRC_DIR}/forward/transparency_oit.frag.slang"
#        "${SHADERS_SRC_DIR}/forward/transparency_oit_composite.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/greyscale.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/bloom_blur.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/ssao_blur.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/fxaa.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/smaa_edge_detect.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/smaa_blend_weight.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/smaa_neighborhood_blend.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/gamma_correction.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/reinhard.frag.slang"
#        "${SHADERS_SRC_DIR}/postprocess/aces.frag.slang"
#        "${SHADERS_SRC_DIR}/shadows/shadowmap.vert.slang"
#        "${SHADERS_SRC_DIR}/shadows/shadowmap.frag.slang"
#        "${SHADERS_SRC_DIR}/shadows/shadowmap_cubemap.frag.slang"
)
add_shaders(${LYSA_TARGET}_shaders ${SHADERS_BUILD_DIR} ${SHADERS_INCLUDE_DIR} ${SHADERS_SOURCE_FILES})

#######################################################
# xxHash library
add_library(xxhash STATIC
        ${xxhash_SOURCE_DIR}/xxhash.c
)
target_include_directories(xxhash PUBLIC
        ${xxhash_SOURCE_DIR}
)
compile_options(xxhash)

#######################################################
if(WIN32)
    set(OS_SRC
            ${ENGINE_SRC_DIR}/os/win32/Application.cpp
            ${ENGINE_SRC_DIR}/os/win32/Input.cpp
            ${ENGINE_SRC_DIR}/os/win32/Window.cpp
    )
    set(OS_MODULES ""
    )
endif ()

#######################################################
if(PHYSIC_ENGINE_JOLT)
    set(PHYSICS_SRC
            ${ENGINE_SRC_DIR}/physics/jolt/Character.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/CollisionArea.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/CollisionObject.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/ConvexHullShape.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/KinematicBody.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/MeshShape.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/PhysicsBody.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/PhysicsEngine.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/PhysicsBody.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/RayCast.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/RigidBody.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/Shape.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/StaticBody.cpp
            ${ENGINE_SRC_DIR}/physics/jolt/StaticCompoundShape.cpp
    )
    set(PHYSICS_MODULES
            ${ENGINE_SRC_DIR}/physics/jolt/PhysicsEngine.ixx
    )
endif ()
if(PHYSIC_ENGINE_PHYSX)
    set(PHYSICS_SRC
            ${ENGINE_SRC_DIR}/physics/physx/Character.cpp
            ${ENGINE_SRC_DIR}/physics/physx/CollisionArea.cpp
            ${ENGINE_SRC_DIR}/physics/physx/CollisionObject.cpp
            ${ENGINE_SRC_DIR}/physics/physx/ConvexHullShape.cpp
            ${ENGINE_SRC_DIR}/physics/physx/KinematicBody.cpp
            ${ENGINE_SRC_DIR}/physics/physx/MeshShape.cpp
            ${ENGINE_SRC_DIR}/physics/physx/PhysicsBody.cpp
            ${ENGINE_SRC_DIR}/physics/physx/PhysicsEngine.cpp
            ${ENGINE_SRC_DIR}/physics/physx/PhysicsBody.cpp
            ${ENGINE_SRC_DIR}/physics/physx/RayCast.cpp
            ${ENGINE_SRC_DIR}/physics/physx/RigidBody.cpp
            ${ENGINE_SRC_DIR}/physics/physx/Shape.cpp
            ${ENGINE_SRC_DIR}/physics/physx/StaticBody.cpp
            ${ENGINE_SRC_DIR}/physics/physx/StaticCompoundShape.cpp
    )
    set(PHYSICS_MODULES
            ${ENGINE_SRC_DIR}/physics/physx/PhysicsEngine.ixx
    )
endif ()

#######################################################
add_library(${LYSA_TARGET} STATIC
        ${ENGINE_SRC_DIR}/AABB.cpp
        ${ENGINE_SRC_DIR}/Application.cpp
        ${ENGINE_SRC_DIR}/AssetsPack.cpp
        ${ENGINE_SRC_DIR}/AsyncQueue.cpp
        ${ENGINE_SRC_DIR}/Frustum.cpp
        ${ENGINE_SRC_DIR}/Global.cpp
        ${ENGINE_SRC_DIR}/Log.cpp
        ${ENGINE_SRC_DIR}/Input.cpp
        ${ENGINE_SRC_DIR}/Math.cpp
        ${ENGINE_SRC_DIR}/Memory.cpp
        ${ENGINE_SRC_DIR}/Loader.cpp
        ${ENGINE_SRC_DIR}/Object.cpp
        ${ENGINE_SRC_DIR}/Resources.cpp
        ${ENGINE_SRC_DIR}/Scene.cpp
        ${ENGINE_SRC_DIR}/Samplers.cpp
        ${ENGINE_SRC_DIR}/Signal.cpp
        ${ENGINE_SRC_DIR}/Viewport.cpp
        ${ENGINE_SRC_DIR}/VirtualFS.cpp
        ${ENGINE_SRC_DIR}/Window.cpp

        ${ENGINE_SRC_DIR}/nodes/AnimationPlayer.cpp
        ${ENGINE_SRC_DIR}/nodes/Camera.cpp
        ${ENGINE_SRC_DIR}/nodes/Character.cpp
        ${ENGINE_SRC_DIR}/nodes/CollisionArea.cpp
        ${ENGINE_SRC_DIR}/nodes/CollisionObject.cpp
        ${ENGINE_SRC_DIR}/nodes/DirectionalLight.cpp
        ${ENGINE_SRC_DIR}/nodes/Environment.cpp
        ${ENGINE_SRC_DIR}/nodes/KinematicBody.cpp
        ${ENGINE_SRC_DIR}/nodes/Light.cpp
        ${ENGINE_SRC_DIR}/nodes/MeshInstance.cpp
        ${ENGINE_SRC_DIR}/nodes/Node.cpp
        ${ENGINE_SRC_DIR}/nodes/OmniLight.cpp
        ${ENGINE_SRC_DIR}/nodes/PhysicsBody.cpp
        ${ENGINE_SRC_DIR}/nodes/RayCast.cpp
        ${ENGINE_SRC_DIR}/nodes/RigidBody.cpp
        ${ENGINE_SRC_DIR}/nodes/SpotLight.cpp
        ${ENGINE_SRC_DIR}/nodes/StaticBody.cpp

        ${ENGINE_SRC_DIR}/pipelines/FrustumCulling.cpp

        ${ENGINE_SRC_DIR}/physics/PhysicsEngine.cpp

        ${ENGINE_SRC_DIR}/resources/Animation.cpp
        ${ENGINE_SRC_DIR}/resources/ConvexHullShape.cpp
        ${ENGINE_SRC_DIR}/resources/Font.cpp
        ${ENGINE_SRC_DIR}/resources/Image.cpp
        ${ENGINE_SRC_DIR}/resources/Material.cpp
        ${ENGINE_SRC_DIR}/resources/Mesh.cpp
        ${ENGINE_SRC_DIR}/resources/MeshShape.cpp
        ${ENGINE_SRC_DIR}/resources/Resource.cpp
        ${ENGINE_SRC_DIR}/resources/Shape.cpp
        ${ENGINE_SRC_DIR}/resources/Texture.cpp

        ${ENGINE_SRC_DIR}/renderers/DebugRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/DeferredRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/ForwardRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/Renderer.cpp
        ${ENGINE_SRC_DIR}/renderers/VectorRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/UIRenderer.cpp

        ${ENGINE_SRC_DIR}/renderers/renderpass/DepthPrepass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/ForwardColor.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/GBufferPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/LightingPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/PostProcessing.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/Renderpass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/ShaderMaterialPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/ShadowMapPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/SMAAPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/SSAOPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/TransparencyPass.cpp

        ${ENGINE_SRC_DIR}/ui/Button.cpp
        ${ENGINE_SRC_DIR}/ui/CheckWidget.cpp
        ${ENGINE_SRC_DIR}/ui/Frame.cpp
        ${ENGINE_SRC_DIR}/ui/Image.cpp
        ${ENGINE_SRC_DIR}/ui/Line.cpp
        ${ENGINE_SRC_DIR}/ui/ScrollBar.cpp
        ${ENGINE_SRC_DIR}/ui/Style.cpp
        ${ENGINE_SRC_DIR}/ui/StyleClassic.cpp
        ${ENGINE_SRC_DIR}/ui/StyleClassicResource.cpp
        ${ENGINE_SRC_DIR}/ui/Text.cpp
        ${ENGINE_SRC_DIR}/ui/TextEdit.cpp
        ${ENGINE_SRC_DIR}/ui/ToggleButton.cpp
        ${ENGINE_SRC_DIR}/ui/TreeView.cpp
        ${ENGINE_SRC_DIR}/ui/ValueSelect.cpp
        ${ENGINE_SRC_DIR}/ui/Widget.cpp
        ${ENGINE_SRC_DIR}/ui/Window.cpp
        ${ENGINE_SRC_DIR}/ui/WindowManager.cpp

        ${OS_SRC}
        ${PHYSICS_SRC}
        ${DEPENDS_SRC_DIR}/stb/stb.cpp
)
target_sources(${LYSA_TARGET}
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
        ${ENGINE_SRC_DIR}/AABB.ixx
        ${ENGINE_SRC_DIR}/Application.ixx
        ${ENGINE_SRC_DIR}/AssetsPack.ixx
        ${ENGINE_SRC_DIR}/AsyncQueue.ixx
        ${ENGINE_SRC_DIR}/Configuration.ixx
        ${ENGINE_SRC_DIR}/Constants.ixx
        ${ENGINE_SRC_DIR}/Enums.ixx
        ${ENGINE_SRC_DIR}/Exception.ixx
        ${ENGINE_SRC_DIR}/Input.ixx
        ${ENGINE_SRC_DIR}/InputEvent.ixx
        ${ENGINE_SRC_DIR}/Frustum.ixx
        ${ENGINE_SRC_DIR}/Global.ixx
        ${ENGINE_SRC_DIR}/Loader.ixx
        ${ENGINE_SRC_DIR}/Log.ixx
        ${ENGINE_SRC_DIR}/Lysa.ixx
        ${ENGINE_SRC_DIR}/Math.ixx
        ${ENGINE_SRC_DIR}/Memory.ixx
        ${ENGINE_SRC_DIR}/Object.ixx
        ${ENGINE_SRC_DIR}/Resources.ixx
        ${ENGINE_SRC_DIR}/Scene.ixx
        ${ENGINE_SRC_DIR}/Samplers.ixx
        ${ENGINE_SRC_DIR}/Signal.ixx
        ${ENGINE_SRC_DIR}/Types.ixx
        ${ENGINE_SRC_DIR}/Tween.ixx
        ${ENGINE_SRC_DIR}/TypeRegistry.ixx
        ${ENGINE_SRC_DIR}/Viewport.ixx
        ${ENGINE_SRC_DIR}/VirtualFS.ixx
        ${ENGINE_SRC_DIR}/Window.ixx

        ${ENGINE_SRC_DIR}/nodes/AnimationPlayer.ixx
        ${ENGINE_SRC_DIR}/nodes/Camera.ixx
        ${ENGINE_SRC_DIR}/nodes/Character.ixx
        ${ENGINE_SRC_DIR}/nodes/CollisionArea.ixx
        ${ENGINE_SRC_DIR}/nodes/CollisionObject.ixx
        ${ENGINE_SRC_DIR}/nodes/DirectionalLight.ixx
        ${ENGINE_SRC_DIR}/nodes/Environment.ixx
        ${ENGINE_SRC_DIR}/nodes/KinematicBody.ixx
        ${ENGINE_SRC_DIR}/nodes/Light.ixx
        ${ENGINE_SRC_DIR}/nodes/MeshInstance.ixx
        ${ENGINE_SRC_DIR}/nodes/Node.ixx
        ${ENGINE_SRC_DIR}/nodes/OmniLight.ixx
        ${ENGINE_SRC_DIR}/nodes/PhysicsBody.ixx
        ${ENGINE_SRC_DIR}/nodes/RayCast.ixx
        ${ENGINE_SRC_DIR}/nodes/RigidBody.ixx
        ${ENGINE_SRC_DIR}/nodes/SpotLight.ixx
        ${ENGINE_SRC_DIR}/nodes/StaticBody.ixx

        ${ENGINE_SRC_DIR}/pipelines/FrustumCulling.ixx

        ${ENGINE_SRC_DIR}/physics/Configuration.ixx
        ${ENGINE_SRC_DIR}/physics/PhysicsEngine.ixx
        ${ENGINE_SRC_DIR}/physics/PhysicsMaterial.ixx

        ${ENGINE_SRC_DIR}/resources/Animation.ixx
        ${ENGINE_SRC_DIR}/resources/AnimationLibrary.ixx
        ${ENGINE_SRC_DIR}/resources/ConvexHullShape.ixx
        ${ENGINE_SRC_DIR}/resources/Font.ixx
        ${ENGINE_SRC_DIR}/resources/Image.ixx
        ${ENGINE_SRC_DIR}/resources/Material.ixx
        ${ENGINE_SRC_DIR}/resources/Mesh.ixx
        ${ENGINE_SRC_DIR}/resources/MeshShape.ixx
        ${ENGINE_SRC_DIR}/resources/Resource.ixx
        ${ENGINE_SRC_DIR}/resources/Shape.ixx
        ${ENGINE_SRC_DIR}/resources/StaticCompoundShape.ixx
        ${ENGINE_SRC_DIR}/resources/Texture.ixx

        ${ENGINE_SRC_DIR}/renderers/DebugRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/DeferredRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/ForwardRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/Renderer.ixx
        ${ENGINE_SRC_DIR}/renderers/UIRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/VectorRenderer.ixx

        ${ENGINE_SRC_DIR}/renderers/renderpass/DepthPrepass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/ForwardColor.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/GBufferPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/LightingPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/PostProcessing.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/Renderpass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/ShaderMaterialPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/ShadowMapPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/SMAAPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/SSAOPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/TransparencyPass.ixx

        ${ENGINE_SRC_DIR}/ui/Box.ixx
        ${ENGINE_SRC_DIR}/ui/Button.ixx
        ${ENGINE_SRC_DIR}/ui/CheckWidget.ixx
        ${ENGINE_SRC_DIR}/ui/Event.ixx
        ${ENGINE_SRC_DIR}/ui/Frame.ixx
        ${ENGINE_SRC_DIR}/ui/Image.ixx
        ${ENGINE_SRC_DIR}/ui/Line.ixx
        ${ENGINE_SRC_DIR}/ui/Panel.ixx
        ${ENGINE_SRC_DIR}/ui/Rect.ixx
        ${ENGINE_SRC_DIR}/ui/Resource.ixx
        ${ENGINE_SRC_DIR}/ui/ScrollBar.ixx
        ${ENGINE_SRC_DIR}/ui/Style.ixx
        ${ENGINE_SRC_DIR}/ui/StyleClassic.ixx
        ${ENGINE_SRC_DIR}/ui/StyleClassicResource.ixx
        ${ENGINE_SRC_DIR}/ui/Text.ixx
        ${ENGINE_SRC_DIR}/ui/TextEdit.ixx
        ${ENGINE_SRC_DIR}/ui/ToggleButton.ixx
        ${ENGINE_SRC_DIR}/ui/TreeView.ixx
        ${ENGINE_SRC_DIR}/ui/ValueSelect.ixx
        ${ENGINE_SRC_DIR}/ui/Widget.ixx
        ${ENGINE_SRC_DIR}/ui/Window.ixx
        ${ENGINE_SRC_DIR}/ui/WindowManager.ixx

        ${OS_MODULES}
        ${PHYSICS_MODULES}
)

compile_options(${LYSA_TARGET})
target_include_directories(${LYSA_TARGET} PUBLIC
        ${INCLUDE_DIR}
        ${HLSLPP_SRC_DIR}/include
        ${DEPENDS_SRC_DIR}/stb
        ${xxhash_SOURCE_DIR}
        ${DEPENDS_SRC_DIR}/json
        ${freetype_SOURCE_DIR}/include
        ${harfbuzz_SOURCE_DIR}/src
        ${msdfgen_SOURCE_DIR}
        ${msdf_atlas_gen_SOURCE_DIR}
)
target_link_libraries(${LYSA_TARGET} ${VIREO_TARGET}
        std-cxx-modules
        xxhash
        Freetype::Freetype
        harfbuzz
)
add_dependencies(${LYSA_TARGET} ${VIREO_TARGET})
set_property(TARGET ${LYSA_TARGET} PROPERTY COMPILE_WARNING_AS_ERROR ON)

#######################################################
if(WIN32)
    target_precompile_headers(${LYSA_TARGET} PRIVATE
            "<intrin.h>"
    )
    target_compile_definitions(${LYSA_TARGET} PRIVATE WIN32_LEAN_AND_MEAN)
    if (DIRECTX_BACKEND)
        target_link_libraries(${LYSA_TARGET} d3d12)
    endif ()
    if(MINGW)
        target_link_options(${LYSA_TARGET} PRIVATE "-mwindows")
    endif()
    set_target_properties(${LYSA_TARGET} PROPERTIES
            WIN32_EXECUTABLE TRUE)
endif()

#######################################################
if(PHYSIC_ENGINE_JOLT)
    target_include_directories(${LYSA_TARGET} PUBLIC ${JoltPhysics_SOURCE_DIR}/..)
    target_link_libraries(${LYSA_TARGET} Jolt)
endif()
if (PHYSIC_ENGINE_PHYSX)
    target_include_directories(${LYSA_TARGET} PUBLIC
        ${PHYSX_ROOT}/include
    )
    target_link_directories(${LYSA_TARGET} PUBLIC ${PHYSX_LIBRARIES})
    target_link_libraries(${LYSA_TARGET}
        PhysX_static_64
        PhysXCharacterKinematic_static_64
        PhysXCommon_static_64
        PhysXCooking_static_64
        PhysXExtensions_static_64
        PhysXFoundation_static_64
        PhysXPvdSDK_static_64
    )
endif()


#######################################################
find_program(DOXYPRESS_EXECUTABLE doxypress)

if (DOXYPRESS_EXECUTABLE)
    message(STATUS "DoxyPress found: ${DOXYPRESS_EXECUTABLE}")
    set(DOCS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs")

    add_custom_target(lysa_docs
            COMMAND ${DOXYPRESS_EXECUTABLE}
            COMMAND ${CMAKE_COMMAND} -E copy
            ${DOCS_DIRECTORY}/navtree.js
            ${DOCS_DIRECTORY}/html/navtree.js
            WORKING_DIRECTORY ${DOCS_DIRECTORY}
            COMMENT "Generating Lysa documentation with DoxyPress"
            VERBATIM
    )
else()
    message(STATUS "DoxyPress not found, doxypress target will not be available.")
endif()