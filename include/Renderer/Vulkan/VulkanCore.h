#pragma once

#include <Core/Core.h>
#include <Debug/Debug.h>
#include <vulkan/vulkan.h>

namespace Aqua
{
    namespace Vulkan
    {
        #ifdef AQUA_DEBUG
            inline constexpr bool ENABLE_VALIDATION_LAYERS = true;
            static std::vector<const char*> VALIDATION_LAYERS = {
                "VK_LAYER_KHRONOS_validation"
            };
        #else
            inline constexpr bool ENABLE_VALIDATION_LAYERS = false;
            static std::vector<const char*> VALIDATION_LAYERS = {};
        #endif

        class Device;
        class Renderer;
        class Buffer;
        class Image;
        class Texture;
    }
}