#pragma once

#include "VulkanCore.h"
#include "VulkanBufferBase.h"
#include "VulkanImage.h"

#include <unordered_set>

namespace Aqua
{
    namespace Vulkan
    {
        class Device
        {
        public:
            struct QueueFamilyIndices
            {
                std::optional<uint32_t> graphics_family;
                std::optional<uint32_t> present_family;

                std::unordered_set<uint32_t> get_unique_family_indices() const
                {
                    std::unordered_set<uint32_t> set;
                    set.reserve(2);

                    if (graphics_family.has_value())
                        set.insert(graphics_family.value());
                    if (present_family.has_value())
                        set.insert(present_family.value());

                    return set;
                }

                bool is_graphics_complete() const noexcept
                {
                    return graphics_family.has_value() &&
                            present_family.has_value();
                }
            };

            Device(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
            ~Device();

            Device(const Device&) = delete;

            VkDevice get_device() const noexcept { return device_; }
            VkPhysicalDevice get_physical_device() const noexcept { return physical_device_; }
            const QueueFamilyIndices& get_queue_families() const noexcept { return queue_families_; }

            VkQueue get_graphics_queue() const noexcept;
            VkQueue get_present_queue() const noexcept;

            VkCommandPool create_command_pool(VkCommandPoolCreateFlags) const;
            VkCommandBuffer create_command_buffer(VkCommandPool command_pool) const;

            void wait_idle() const { vkDeviceWaitIdle(device_); }
            void wait_queue_idle(VkQueue queue) const { vkQueueWaitIdle(queue); }

            void submit_command_buffer(VkQueue queue, VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE) const;

            void submit_graphics_command_buffer(VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE) const
            {
                submit_command_buffer(get_graphics_queue(), command_buffer, fence);
            }

            void submit_present_command_buffer(VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE) const
            {
                submit_command_buffer(get_present_queue(), command_buffer, fence);
            }

            template<typename FUNC>
            requires requires (FUNC&& func)
            { std::is_same_v<decltype(std::function(std::forward<FUNC>(func))), std::function<void(VkCommandBuffer)>>; }
            void submit_one_time_commands(FUNC&& commands) const
            {
                auto cmd_set = begin_single_time_commands();
                commands(cmd_set.second);
                end_single_time_commands(cmd_set);
            }

            std::pair<VkCommandPool, VkCommandBuffer> begin_single_time_commands() const;
            void end_single_time_commands(const std::pair<VkCommandPool, VkCommandBuffer>& buffer) const;

            Buffer create_buffer(VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties) const;

            Image create_image(const VkImageCreateInfo& info,
                               VkMemoryPropertyFlags properties) const;

            static QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

        private:
            VkDevice device_ = VK_NULL_HANDLE;
            VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
            QueueFamilyIndices queue_families_;
            // VkSurface surface_ associated_surface_ = VK_NULL_HANDLE;
            
            static std::vector<const char*> device_extensions_;
            static VkDevice create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

            static VkCommandPool create_graphics_command_pool(VkDevice device,
                                                            const QueueFamilyIndices& queue_family_indices,
                                                            VkCommandPoolCreateFlags flags);

            static VkCommandBuffer create_command_buffer(VkDevice device, VkCommandPool command_pool);
            static uint32_t get_memory_type(VkPhysicalDevice physical_device,
                                            uint32_t type_filter,
                                            VkMemoryPropertyFlags properties);

            static Buffer create_device_buffer(const Device& device,
                                               VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VkMemoryPropertyFlags properties);
            
            static Image create_device_image(const Device& device,
                                             const VkImageCreateInfo& info,
                                             VkMemoryPropertyFlags properties);
        };
    }
}