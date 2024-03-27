#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <stdint.h>
#include <vector>
#include <optional>
#include <filesystem>

#include "Platform.h"

#ifdef AQUA_SHARED_API
    #ifdef AQUA_PLATFORM_WINDOWS
        #ifdef AQUA_BUILD_SHARED_API
            #define AQUA_API __declspec(dllexport)
        #else
            #define AQUA_API __declspec(dllimport)
        #endif
    #endif
#else
    #define AQUA_API
#endif

#ifdef AQUA_PLATFORM_WINDOWS
    #define NOMINMAX
#endif