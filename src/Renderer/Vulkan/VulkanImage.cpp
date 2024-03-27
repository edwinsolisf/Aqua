#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanDevice.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

namespace Aqua
{
    namespace Vulkan
    {
        Image::Image(VkDevice device, VkImage image, VkDeviceMemory memory, VkFormat format, VkExtent3D size)
            : device_{ device }, image_{ image }, memory_{ memory }, format_{ format }, size_{ size }
        {
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = image;
            view_info.format = format;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;

            if (vkCreateImageView(device, &view_info, nullptr, &view_) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create image view");
        }

        Image::~Image()
        {
            if (device_ != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device_, view_, nullptr);
                vkFreeMemory(device_, memory_, nullptr);
                vkDestroyImage(device_, image_, nullptr);
            }

            memory_ = VK_NULL_HANDLE;
            image_ = VK_NULL_HANDLE;
        }

        Image::Image(Image&& other) noexcept
            : memory_{ std::exchange(other.memory_, VK_NULL_HANDLE) },
              image_{ std::exchange(other.image_, VK_NULL_HANDLE) },
              device_{ std::exchange(other.device_, VK_NULL_HANDLE) },
              view_{ std::exchange(other.view_, VK_NULL_HANDLE) },
              format_{other.format_}, size_{other.size_}, layout_{other.layout_}
        {
        }
            
        Image& Image::operator=(Image&& other) noexcept
        {
            memory_ = std::exchange(other.memory_, VK_NULL_HANDLE);
            image_ = std::exchange(other.image_, VK_NULL_HANDLE);
            device_ = std::exchange(other.device_, VK_NULL_HANDLE);
            view_ = std::exchange(other.view_, VK_NULL_HANDLE);
            format_ = other.format_;
            size_ = other.size_;
            layout_ = other.layout_;

            return *this;
        }

        void Image::transition_image_layout(const Device& device, Image& image, VkImageLayout new_layout)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = image.get_layout();
            barrier.newLayout = new_layout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image.get_image();
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
    
            VkPipelineStageFlags source_stage{};
            VkPipelineStageFlags destination_stage{};
            if (image.get_layout() == VK_IMAGE_LAYOUT_UNDEFINED
                && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            else if (image.get_layout() == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            }
            device.submit_one_time_commands(
                [&](VkCommandBuffer command_buffer)
                {
                    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0,
                        0, nullptr, 0, nullptr, 1, &barrier);
                });
            
            image.layout_ = new_layout;
        }

        void Image::copy_from_buffer(const Device& device, const Buffer& src, const Image& dst)
        {

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageExtent = { dst.get_width(), dst.get_height(), dst.get_depth() };
            region.imageOffset = { 0 , 0 , 0 };
            
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.layerCount = 1;
            region.imageSubresource.baseArrayLayer = 0;

            device.submit_one_time_commands(
                [&](VkCommandBuffer command_buffer)
                {
                    vkCmdCopyBufferToImage(command_buffer, src.get_buffer(), dst.get_image(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                });
        }
    }
}
