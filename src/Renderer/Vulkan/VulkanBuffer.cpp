#include "Renderer/Vulkan/VulkanBuffer.h"

namespace Aqua
{
    namespace Vulkan
    {        
        void VertexBuffer::create_vertex_buffer(const Device& device, const void* src_data, VkDeviceSize size)
        {
            auto buffer_size = get_buffer_size();
            auto stage_buffer = device.create_buffer(
                                    buffer_size,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            void* dst_data = nullptr;
            vkMapMemory(device.get_device(), stage_buffer.get_memory(), 0, get_buffer_size(), 0, &dst_data);

            if (dst_data)
                std::copy(reinterpret_cast<const uint8_t*>(src_data),
                          reinterpret_cast<const uint8_t*>(src_data) + size,
                          reinterpret_cast<uint8_t*>(dst_data));
            else
            {
                AQUA_ERROR("Vulkan Error: failed to map DEVICE memory to HOST memory");
                return;
            }

            vkUnmapMemory(device.get_device(), stage_buffer.get_memory());
            
            Buffer::copy(device, stage_buffer, *this, buffer_size);
        }

        void VertexBuffer::bind_buffer(VkCommandBuffer command_buffer) const
        {
            VkBuffer vertex_buffers[] = { buffer_ };
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        }

        IndexBuffer::IndexBuffer(const Device& device,
                                 const std::vector<uint32_t>& indices)
            : Buffer{device.create_buffer(static_cast<uint32_t>(indices.size()) * sizeof(indices),
                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) },
              index_count_{ static_cast<uint32_t>(indices.size()) }
        {
            auto buffer_size = get_buffer_size();

            auto stage_buffer = device.create_buffer(buffer_size,
                                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            void* data = nullptr;
            vkMapMemory(device_, stage_buffer.get_memory(), 0, buffer_size, 0, &data);
            std::copy(indices.cbegin(), indices.cend(), static_cast<uint32_t*>(data));
            vkUnmapMemory(device_, stage_buffer.get_memory());

            Buffer::copy(device, stage_buffer, *this, buffer_size);
        }

        void IndexBuffer::bind_buffer(VkCommandBuffer command_buffer) const
        {
            vkCmdBindIndexBuffer(command_buffer, buffer_, 0, VK_INDEX_TYPE_UINT32);
        }

        void UniformBuffer::create_uniform_buffer(const Device& device, const void* data)
        {
            vkMapMemory(device_, memory_, 0, buffer_size_, 0, &mapped_memory_);
        }

        void UniformBuffer::bind_buffer(VkCommandBuffer command_buffer) const
        {
        }
    }
}