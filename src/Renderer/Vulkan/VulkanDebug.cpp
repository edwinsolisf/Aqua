#include "Renderer/Vulkan/VulkanDebug.h"

namespace Aqua
{
    namespace Vulkan
    {
        VkResult create_debug_utils_messenger_ext(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
            const VkAllocationCallbacks* p_allocator,
            VkDebugUtilsMessengerEXT* p_debug_messenger)
        {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

            if (func != nullptr)
                return func(instance, p_create_info, p_allocator, p_debug_messenger);
            else
                return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        void destroy_debug_utils_messenger_ext(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debug_messenger,
            const VkAllocationCallbacks* p_allocator)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

            if (func != nullptr)
                return func(instance, debug_messenger, p_allocator);
        }


        VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
        {
            std::string output = "[Vulkan Debug](";

            if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
                output += "General";
            if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
                output += "Performance";
            if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
                output += "Validation";
            
            output += ") ";
            output += callback_data->pMessage;

            if (message_severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                AQUA_INFO(output);
            else if (message_severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                AQUA_WARN(output);
            else
                AQUA_ERROR(output);

            return VK_FALSE;
        }
    }
}