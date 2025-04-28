#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef UNICODE
        #define UNICODE
    #endif
    #ifndef _UNICODE
        #define _UNICODE
    #endif
    #include <windows.h>
#endif

#include <cstdint>
#include <cstddef>

import std;
import glm;
import vireo;

using namespace std;
using namespace glm;
using namespace vireo;
