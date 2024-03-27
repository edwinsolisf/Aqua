#pragma once

#include "VulkanCore.h"

namespace Aqua
{
    namespace Vulkan
    {
        class Image
        {
        public:
            ~Image();

            uint32_t get_width() const noexcept { return size_.width; }
            uint32_t get_height() const noexcept { return size_.height; }
            uint32_t get_depth() const noexcept { return size_.depth; }

            VkImage get_image() const noexcept { return image_; }
            VkDeviceMemory get_memory() const noexcept { return memory_; }
            VkDevice get_device() const noexcept { return device_; }
            VkFormat get_format() const noexcept { return format_; }
            VkImageLayout get_layout() const noexcept { return layout_; }
            VkImageView get_view() const noexcept { return view_; }

            static void copy_from_buffer(const Device& device, const Buffer& src, const Image& dst);
            static void transition_image_layout(const Device& device, Image& image, VkImageLayout new_layout);
            
        private:
            Image() = default;
            Image(const Image&) = delete;
            Image& operator=(const Image&) = delete;

            Image(Image&&) noexcept;
            Image& operator=(Image&&) noexcept;

            Image(VkDevice device, VkImage image, VkDeviceMemory memory, VkFormat format, VkExtent3D size);
            
            VkImage image_ = VK_NULL_HANDLE;
            VkImageView view_ = VK_NULL_HANDLE;
            VkDeviceMemory memory_ = VK_NULL_HANDLE;
            VkDevice device_ = VK_NULL_HANDLE;
            VkFormat format_;
            VkExtent3D size_;
            VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;

            friend class Device;
            friend class Texture;
        };
    }
}