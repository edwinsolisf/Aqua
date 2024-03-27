#include "Renderer/Vulkan/VulkanDevice.h"

namespace Aqua
{
    namespace Vulkan
    {
        std::vector<const char*> Device::device_extensions_ = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        Device::Device(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
        {
            physical_device_ = physical_device;
            queue_families_ = find_queue_families(physical_device, surface);
            device_ = create_logical_device(physical_device, surface);

            AQUA_INFO("Created Vulkan Device");
        }

        Device::~Device()
        {
            vkDestroyDevice(device_, nullptr);
        }

        VkQueue Device::get_graphics_queue() const noexcept
        {
            VkQueue queue = VK_NULL_HANDLE;

            if (queue_families_.graphics_family.has_value())
                vkGetDeviceQueue(device_, queue_families_.graphics_family.value(), 0, &queue);

            return queue;
        }
            
        VkQueue Device::get_present_queue() const noexcept
        {
            VkQueue queue = VK_NULL_HANDLE;

            if (queue_families_.present_family.has_value())
                vkGetDeviceQueue(device_, queue_families_.present_family.value(), 0, &queue);

            return queue;
        }

        VkCommandPool Device::create_command_pool(VkCommandPoolCreateFlags flags) const
        {
            AQUA_INFO("Created command pool");
            return create_graphics_command_pool(device_, queue_families_, flags);
        }
            
        VkCommandBuffer Device::create_command_buffer(VkCommandPool command_pool) const
        {
            AQUA_INFO("Created command buffer");
            return create_command_buffer(device_, command_pool);
        }

        void Device::submit_command_buffer(VkQueue queue, VkCommandBuffer command_buffer, VkFence fence) const
        {
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            if (vkQueueSubmit(queue, 1, &submit_info, fence) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to submit command");
        }

        std::pair<VkCommandPool, VkCommandBuffer> Device::begin_single_time_commands() const
        {
            VkCommandPool command_pool = create_command_pool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
            VkCommandBuffer command_buffer = create_command_buffer(command_pool);

            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(command_buffer, &begin_info);

            return { command_pool , command_buffer };
        }
        
        void Device::end_single_time_commands(const std::pair<VkCommandPool, VkCommandBuffer>& command_set) const
        {
            auto command_pool = command_set.first;
            auto command_buffer = command_set.second;

            vkEndCommandBuffer(command_buffer);

            auto queue = get_graphics_queue();
            submit_command_buffer(queue, command_buffer);
            wait_queue_idle(queue);

            vkFreeCommandBuffers(device_, command_pool, 1, &command_buffer);
            vkDestroyCommandPool(device_, command_pool, nullptr);
        }

        Buffer Device::create_buffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties) const
        {
            return create_device_buffer(*this, size, usage, properties);
        }

        Image Device::create_image(
            const VkImageCreateInfo& info,
            VkMemoryPropertyFlags properties) const
        {
            return create_device_image(*this, info, properties);
        }

        Device::QueueFamilyIndices Device::find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            QueueFamilyIndices indices;
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

            uint32_t i = 0;
            for (const auto& queue_family : queue_families)
            {
                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.graphics_family = i;
                if (present_support)
                    indices.present_family = i;
                ++i;
            }

            return indices;
        }

        VkDevice Device::create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
        {
            VkDevice device = VK_NULL_HANDLE;

            QueueFamilyIndices indices = find_queue_families(physical_device, surface);
            std::unordered_set<uint32_t> unique_queue_families = indices.get_unique_family_indices();

            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            queue_create_infos.reserve(unique_queue_families.size());

            float queue_priority = 1.0f;
            for (auto queue_family : unique_queue_families)
            {
                VkDeviceQueueCreateInfo queue_info{};
                queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queue_info.queueFamilyIndex = queue_family;
                queue_info.queueCount = 1;
                queue_info.pQueuePriorities = &queue_priority;
                
                queue_create_infos.push_back(queue_info);
            }

            VkPhysicalDeviceFeatures features{};

            VkDeviceCreateInfo device_info{};
            device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_info.pEnabledFeatures = &features;
            device_info.pQueueCreateInfos = queue_create_infos.data();
            device_info.queueCreateInfoCount = queue_create_infos.size();
            device_info.enabledExtensionCount = 0;

            if (::Aqua::Vulkan::ENABLE_VALIDATION_LAYERS)
            {
                device_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
                device_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
            }
            else
            {
                device_info.enabledLayerCount = 0;
                device_info.ppEnabledLayerNames = nullptr;
            }

            device_info.ppEnabledExtensionNames = device_extensions_.data();
            device_info.enabledExtensionCount = device_extensions_.size();
            
            if (vkCreateDevice(physical_device, &device_info, nullptr, &device))
                AQUA_ERROR("Vulkan Error: Failed to create logical device");

            return device;
        }

        VkCommandPool Device::create_graphics_command_pool(VkDevice device,
                                                            const QueueFamilyIndices& queue_family_indices,
                                                            VkCommandPoolCreateFlags flags)
        {
            VkCommandPool command_pool = VK_NULL_HANDLE;

            VkCommandPoolCreateInfo pool_info{};
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.flags = flags;

            if (queue_family_indices.graphics_family.has_value())
            {
                pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
            }
            else
            {
                pool_info.queueFamilyIndex = 0;

                AQUA_ERROR("Vulkan Error: failed to create graphics command pool on device does not support graphics queue");
            }

            if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create graphics command pool");

            return command_pool;
        }

        VkCommandBuffer Device::create_command_buffer(VkDevice device, VkCommandPool command_pool)
        {
            VkCommandBuffer command_buffer = VK_NULL_HANDLE;

            VkCommandBufferAllocateInfo allocate_info{};
            allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocate_info.commandPool = command_pool;
            allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocate_info.commandBufferCount = 1;

            if (vkAllocateCommandBuffers(device, &allocate_info, &command_buffer) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to allocate command buffers");
            
            return command_buffer;
        }

        uint32_t Device::get_memory_type(VkPhysicalDevice physical_device,
                                            uint32_t type_filter,
                                            VkMemoryPropertyFlags properties)
        {
            VkPhysicalDeviceMemoryProperties memory_properties{};
            vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

            for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
            {
                if ((type_filter & (1 << i)) &&
                    (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }

            AQUA_ERROR("Vulkan Error: failed to find suitable memory type");

            return 0;
        }

        Buffer Device::create_device_buffer(
            const Device& device,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties)
        {
            VkBuffer buffer = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            VkBufferCreateInfo buffer_info{};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.usage = usage;
            buffer_info.size = size;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device.get_device(), &buffer_info, nullptr, &buffer) != VK_SUCCESS)
            {
                AQUA_ERROR("Vulkan Error: failed to create buffer");
            }

            VkMemoryRequirements memory_requirements{};
            vkGetBufferMemoryRequirements(device.get_device(), buffer, &memory_requirements);

            VkMemoryAllocateInfo allocate_info{};
            allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocate_info.allocationSize = memory_requirements.size;
            allocate_info.memoryTypeIndex = get_memory_type(device.get_physical_device(),
                                                            memory_requirements.memoryTypeBits,
                                                            properties);

            if (vkAllocateMemory(device.get_device(), &allocate_info, nullptr, &memory) != VK_SUCCESS)
            {
                vkDestroyBuffer(device.get_device(), buffer, nullptr);
                buffer = VK_NULL_HANDLE;

                AQUA_ERROR("Vulkan Error: failed to allocate memory for buffer");
            }

            if (vkBindBufferMemory(device.get_device(), buffer, memory, 0) != VK_SUCCESS)
            {
                vkDestroyBuffer(device.get_device(), buffer, nullptr);
                vkFreeMemory(device.get_device(), memory, nullptr);

                buffer = VK_NULL_HANDLE;
                memory = VK_NULL_HANDLE;

                AQUA_ERROR("Vulkan Error: failed to bind memory to buffer");
            }

            return { device.get_device(), buffer , memory, size };
        }

        Image Device::create_device_image(
            const Device& device,
            const VkImageCreateInfo& info,
            VkMemoryPropertyFlags properties)
        {
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            if (vkCreateImage(device.get_device(), &info, nullptr, &image) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create image");

            VkMemoryRequirements memory_requirements{};
            vkGetImageMemoryRequirements(device.get_device(), image, &memory_requirements);

            VkMemoryAllocateInfo allocate_info{};
            allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocate_info.allocationSize = memory_requirements.size;
            allocate_info.memoryTypeIndex = get_memory_type(device.get_physical_device(),
                                                            memory_requirements.memoryTypeBits,
                                                            properties);

            if (vkAllocateMemory(device.get_device(), &allocate_info, nullptr, &memory) != VK_SUCCESS)
            {
                vkDestroyImage(device.get_device(), image, nullptr);
                image = VK_NULL_HANDLE;

                AQUA_ERROR("Vulkan Error: failed to allocate memory for buffer");
            }

            if (vkBindImageMemory(device.get_device(), image, memory, 0) != VK_SUCCESS)
            {
                vkFreeMemory(device.get_device(), memory, nullptr);
                vkDestroyImage(device.get_device(), image, nullptr);
                memory = VK_NULL_HANDLE;
                image = VK_NULL_HANDLE;

                AQUA_ERROR("Vulkan Error: failed to bind memory to image");
            }

            return { device.get_device(), image , memory, info.format, info.extent };
        }
    }
}