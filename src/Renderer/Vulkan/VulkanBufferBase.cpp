#include "Renderer/Vulkan/VulkanBufferBase.h"
#include "Renderer/Vulkan/VulkanDevice.h"

namespace Aqua
{
    namespace Vulkan
    {
        bool Buffer::write_data(const uint8_t* src_data, VkDeviceSize size, VkDeviceSize dst_offset) const
        {
            if (get_buffer_size() > (dst_offset + size))
            {
                AQUA_ERROR("Vulkan Error: writing data outside of buffer memory");
                return false;
            }

            void* dst_data = nullptr;
            vkMapMemory(device_, memory_, dst_offset, size, 0, &dst_data);

            if (!dst_data)
            {
                AQUA_WARN("Vulkan Warning: failed to map device memory to host memory");
                return false;
            }

            std::copy(src_data, src_data + size, reinterpret_cast<uint8_t*>(dst_data));
            vkUnmapMemory(device_, memory_);

            return true;
        }

        void Buffer::copy(const Device& device,
                          const Buffer& src_buffer,
                          const Buffer& dst_buffer,
                          VkDeviceSize size,
                          VkDeviceSize src_offset,
                          VkDeviceSize dst_offset)
        {
            if (src_buffer.get_buffer_size() > (src_offset + size))
            {
                AQUA_ERROR("Vulkan Error: copying memory outside of source buffer");
                return;
            }

            if (dst_buffer.get_buffer_size() > (dst_offset + size))
            {
                AQUA_ERROR("Vulkan Error: copying memory outside of destination buffer");
                return;
            }

            device.submit_one_time_commands(
                [&](VkCommandBuffer command_buffer)
                {
                    VkBufferCopy copy_region{};
                    copy_region.size = size;
                    copy_region.dstOffset = dst_offset;
                    copy_region.srcOffset = src_offset;

                    vkCmdCopyBuffer(command_buffer, src_buffer.get_buffer(), dst_buffer.get_buffer(), 1, &copy_region);
                });
        }
    }
}