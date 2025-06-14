#
# Copyright (c) 2024-2025 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
########################################################################################################################
if(PHYSIC_ENGINE_JOLT)
    include(cmake/jolt.cmake)
endif()

###### xxHash
message(NOTICE "Fetching xxHash from https://github.com/Cyan4973/xxHash...")
FetchContent_Declare(
        xxhash
        GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
        GIT_TAG v0.8.3
)
set(XXHASH_BUNDLED_MODE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(xxhash)