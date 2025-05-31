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

set(LYSA_TARGET ${PROJECT_NAME})
#set(DIRECTX_BACKEND ON)

#######################################################
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
endif ()


#######################################################
include(FetchContent)
include(cmake/shaders.cmake)
include(cmake/libraries.cmake)

#######################################################
add_subdirectory(${VIREO_PROJECT_DIR} external_lib_build)
set(VIREO_TARGET "vireo_rhi")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ENGINE_SRC_DIR ${SRC_DIR}/${LYSA_TARGET})
set(SHADERS_SRC_DIR ${ENGINE_SRC_DIR}/shaders)
set(SHADERS_INCLUDE_DIR ${ENGINE_SRC_DIR}/shaders/include)
set(SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(HLSLPP_TARGET hlslpp)
set(HLSLPP_SRC_DIR ${SRC_DIR}/${HLSLPP_TARGET})

#######################################################
# Slang shaders
file(MAKE_DIRECTORY ${SHADERS_BUILD_DIR})
#file(GLOB_RECURSE SHADERS_SOURCE_FILES
#        "${SHADERS_SRC_DIR}/*.slang"
#)
set(SHADERS_SOURCE_FILES
        "${SHADERS_SRC_DIR}/default.vert.slang"
        "${SHADERS_SRC_DIR}/forward.frag.slang"
        "${SHADERS_SRC_DIR}/gamma_correction.frag.slang"
        "${SHADERS_SRC_DIR}/quad.vert.slang"
)
add_shaders(${LYSA_TARGET}_shaders ${SHADERS_BUILD_DIR} ${SHADERS_INCLUDE_DIR} ${SHADERS_SOURCE_FILES} )

#set(SHADERS_SOURCE_FILES_FORWARD
#        "${SHADERS_SRC_DIR}/default.vert.slang"
#)
#add_shaders(${LYSA_TARGET}_shaders_forward ${SHADERS_BUILD_DIR} ${SHADERS_INCLUDE_DIR} ${SHADERS_SOURCE_FILES_FORWARD} )

#######################################################
# xxHash library
add_library(xxhash STATIC
        ${xxhash_SOURCE_DIR}/xxhash.c
)

target_include_directories(xxhash PUBLIC
        ${xxhash_SOURCE_DIR}
)

#######################################################
# HLSL++ math library
add_library(${HLSLPP_TARGET})
target_sources(${HLSLPP_TARGET}
        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        ${HLSLPP_SRC_DIR}/module/hlsl++.ixx
)
compile_options(${HLSLPP_TARGET})
target_include_directories(${HLSLPP_TARGET} PUBLIC ${HLSLPP_SRC_DIR}/include)
target_link_libraries(${HLSLPP_TARGET} std-cxx-modules)

#######################################################
if(WIN32)
    set(WIN32_SRC
            ${ENGINE_SRC_DIR}/os/win32/Application.cpp
            ${ENGINE_SRC_DIR}/os/win32/Input.cpp
            ${ENGINE_SRC_DIR}/os/win32/Window.cpp
    )
    set(WIN32_MODULES ""
    )
endif ()

add_library(${LYSA_TARGET} STATIC
        ${ENGINE_SRC_DIR}/AABB.cpp
        ${ENGINE_SRC_DIR}/Application.cpp
        ${ENGINE_SRC_DIR}/AssetsPack.cpp
        ${ENGINE_SRC_DIR}/Global.cpp
        ${ENGINE_SRC_DIR}/Log.cpp
        ${ENGINE_SRC_DIR}/Input.cpp
        ${ENGINE_SRC_DIR}/Memory.cpp
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
        ${ENGINE_SRC_DIR}/nodes/Environment.cpp
        ${ENGINE_SRC_DIR}/nodes/MeshInstance.cpp
        ${ENGINE_SRC_DIR}/nodes/Node.cpp

        ${ENGINE_SRC_DIR}/resources/Animation.cpp
        ${ENGINE_SRC_DIR}/resources/Image.cpp
        ${ENGINE_SRC_DIR}/resources/Material.cpp
        ${ENGINE_SRC_DIR}/resources/Mesh.cpp
        ${ENGINE_SRC_DIR}/resources/Resource.cpp
        ${ENGINE_SRC_DIR}/resources/Texture.cpp

        ${ENGINE_SRC_DIR}/renderers/ForwardRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/Renderer.cpp

        ${ENGINE_SRC_DIR}/renderers/renderpass/ForwardColor.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/PostProcessing.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpass/Renderpass.cpp

        ${WIN32_SRC}
        ${SRC_DIR}/stb/stb.cpp
)
target_sources(${LYSA_TARGET}
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
        ${ENGINE_SRC_DIR}/AABB.ixx
        ${ENGINE_SRC_DIR}/Application.ixx
        ${ENGINE_SRC_DIR}/AssetsPack.ixx
        ${ENGINE_SRC_DIR}/Configuration.ixx
        ${ENGINE_SRC_DIR}/Constants.ixx
        ${ENGINE_SRC_DIR}/Enums.ixx
        ${ENGINE_SRC_DIR}/Input.ixx
        ${ENGINE_SRC_DIR}/InputEvent.ixx
        ${ENGINE_SRC_DIR}/Global.ixx
        ${ENGINE_SRC_DIR}/Log.ixx
        ${ENGINE_SRC_DIR}/Lysa.ixx
        ${ENGINE_SRC_DIR}/Memory.ixx
        ${ENGINE_SRC_DIR}/Object.ixx
        ${ENGINE_SRC_DIR}/Resources.ixx
        ${ENGINE_SRC_DIR}/Scene.ixx
        ${ENGINE_SRC_DIR}/Samplers.ixx
        ${ENGINE_SRC_DIR}/Signal.ixx
        ${ENGINE_SRC_DIR}/Types.ixx
        ${ENGINE_SRC_DIR}/Viewport.ixx
        ${ENGINE_SRC_DIR}/VirtualFS.ixx
        ${ENGINE_SRC_DIR}/Window.ixx

        ${ENGINE_SRC_DIR}/nodes/AnimationPlayer.ixx
        ${ENGINE_SRC_DIR}/nodes/Camera.ixx
        ${ENGINE_SRC_DIR}/nodes/Environment.ixx
        ${ENGINE_SRC_DIR}/nodes/MeshInstance.ixx
        ${ENGINE_SRC_DIR}/nodes/Node.ixx

        ${ENGINE_SRC_DIR}/resources/Animation.ixx
        ${ENGINE_SRC_DIR}/resources/AnimationLibrary.ixx
        ${ENGINE_SRC_DIR}/resources/Image.ixx
        ${ENGINE_SRC_DIR}/resources/Material.ixx
        ${ENGINE_SRC_DIR}/resources/Mesh.ixx
        ${ENGINE_SRC_DIR}/resources/Resource.ixx
        ${ENGINE_SRC_DIR}/resources/Texture.ixx

        ${ENGINE_SRC_DIR}/renderers/ForwardRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/Renderer.ixx

        ${ENGINE_SRC_DIR}/renderers/renderpass/ForwardColor.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/PostProcessing.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpass/Renderpass.ixx
        ${WIN32_MODULES}
)

compile_options(${LYSA_TARGET})
target_include_directories(${LYSA_TARGET} PUBLIC ${INCLUDE_DIR} ${SRC_DIR}/stb ${xxhash_SOURCE_DIR})
target_link_libraries(${LYSA_TARGET} ${VIREO_TARGET} std-cxx-modules ${HLSLPP_TARGET} xxhash)
add_dependencies(${LYSA_TARGET} ${VIREO_TARGET})
target_precompile_headers(${LYSA_TARGET} PRIVATE
        "<cstdint>"
)
if(WIN32)
    if (DIRECTX_BACKEND)
        target_link_libraries(${LYSA_TARGET} d3d12)
    endif ()
    if(MINGW)
        target_link_options(${LYSA_TARGET} PRIVATE "-mwindows")
    endif()
    set_target_properties(${LYSA_TARGET} PROPERTIES
            WIN32_EXECUTABLE TRUE)
endif()