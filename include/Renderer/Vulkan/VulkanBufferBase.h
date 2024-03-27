#pragma once

#include "VulkanCore.h"

namespace Aqua
{
    namespace Vulkan
    {
        class Buffer
        {
        public:
            ~Buffer()
            {
                vkFreeMemory(device_, memory_, nullptr);
                vkDestroyBuffer(device_, buffer_, nullptr);
            }

            bool write_data(const uint8_t* src_data, VkDeviceSize size, VkDeviceSize dst_offset = 0) const;

            VkDeviceSize get_buffer_size() const noexcept { return buffer_size_; }
            VkBuffer get_buffer() const noexcept { return buffer_; }
            VkDeviceMemory get_memory() const noexcept { return memory_; }
            VkDevice get_device() const noexcept { return device_; }

            static void copy(const Device& device, const Buffer& src_buffer, const Buffer& dst_buffer, VkDeviceSize size,
                            VkDeviceSize src_offset = 0, VkDeviceSize dst_offset = 0);
        protected:
            Buffer() = default;
            Buffer(const Buffer&) = delete;
            // Buffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
            Buffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size)
                : buffer_{ buffer }, memory_{ memory }, device_{ device }, buffer_size_{ size } {}

            VkBuffer buffer_ = VK_NULL_HANDLE;
            VkDeviceMemory memory_ = VK_NULL_HANDLE;
            VkDevice device_ = VK_NULL_HANDLE;
            VkDeviceSize buffer_size_ = 0;

            friend class Device;
        };
    }
}