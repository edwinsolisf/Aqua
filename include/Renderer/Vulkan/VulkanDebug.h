#pragma once

#include "VulkanCore.h"

namespace Aqua
{
    namespace Vulkan
    {
        VkResult create_debug_utils_messenger_ext(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
            const VkAllocationCallbacks* p_allocator,
            VkDebugUtilsMessengerEXT* p_debug_messenger);

        void destroy_debug_utils_messenger_ext(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debug_messenger,
            const VkAllocationCallbacks* p_allocator);

        VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data);
    }
}
