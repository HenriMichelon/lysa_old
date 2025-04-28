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
set(DIRECTX_BACKEND ON)

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
set(VIREO_INCLUDE_DIR ${VIREO_PROJECT_DIR}/include)
set(VIREO_TARGET "vireo_rhi")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ENGINE_SRC_DIR ${SRC_DIR}/${LYSA_TARGET})

#######################################################
add_library(${LYSA_TARGET} STATIC
        ${ENGINE_SRC_DIR}/Lysa.cpp
)
target_sources(${LYSA_TARGET}
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
        ${ENGINE_SRC_DIR}/Lysa.ixx
        ${ENGINE_SRC_DIR}/Signal.ixx
)
compile_options(${LYSA_TARGET})
target_include_directories(${LYSA_TARGET} PUBLIC ${INCLUDE_DIR} ${VIREO_INCLUDE_DIR})
target_link_libraries(${LYSA_TARGET} ${VIREO_TARGET} glm::glm glm-modules std-cxx-modules)
add_dependencies(${LYSA_TARGET} ${VIREO_TARGET})
if(WIN32)
    target_precompile_headers(${LYSA_TARGET} PRIVATE
            "<windows.h>"
            "<cstdint>"
            "<cstddef>"
    )
    if (DIRECTX_BACKEND)
        target_link_libraries(${LYSA_TARGET} Xinput dinput8 dxguid dxgi d3d12 d3dcompiler)
    endif ()
    if(MINGW)
        target_link_options(${LYSA_TARGET} PRIVATE "-mwindows")
    endif()
    set_target_properties(${LYSA_TARGET} PROPERTIES
            WIN32_EXECUTABLE TRUE)
endif()