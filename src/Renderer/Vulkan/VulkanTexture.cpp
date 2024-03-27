#include "Renderer/Vulkan/VulkanTexture.h"

#include <stb/stb_image.h>
// #include "Renderer/Vulkan/"

namespace Aqua
{
    namespace Vulkan
    {
        Texture::Texture(const Device& device, const std::filesystem::path& filepath)
        {
            int width = 0, height = 0, channels = 0;
            const uint32_t image_channels = 4;
            
            auto file = filepath.string();
            stbi_uc* image_data = stbi_load(file.c_str(), &width, &height, &channels, image_channels);
            VkDeviceSize image_size = width * height * image_channels;

            VkImageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.imageType = VK_IMAGE_TYPE_2D;
            info.extent = {.width  = static_cast<uint32_t>(width),
                           .height = static_cast<uint32_t>(height),
                           .depth  = 1};
            info.mipLevels = 1;
            info.arrayLayers = 1;
            info.format = VK_FORMAT_R8G8B8A8_SRGB;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.flags = 0;

            image_ = std::move(device.create_image(info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

            auto stage_buffer = device.create_buffer(image_size,
                                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (!stage_buffer.write_data(image_data, image_size))
            {
                AQUA_ERROR("Vulkan Error: failed to store data");
                return;
            }

            Image::transition_image_layout(device, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            Image::copy_from_buffer(device, stage_buffer, image_);
            Image::transition_image_layout(device, image_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            stbi_image_free(image_data);

            VkSamplerCreateInfo sampler_info{};
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.minFilter = VK_FILTER_LINEAR;
            sampler_info.magFilter = VK_FILTER_LINEAR;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

            sampler_info.anisotropyEnable = VK_FALSE;
            sampler_info.maxAnisotropy = 0.f;

            sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler_info.unnormalizedCoordinates = VK_FALSE;
            sampler_info.compareEnable = VK_FALSE;
            sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
            sampler_info.minLod = 0.f;
            sampler_info.maxLod = 0.f;
            sampler_info.mipLodBias = 0.f;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            
            if (vkCreateSampler(device.get_device(), &sampler_info, nullptr, &sampler_) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create texture sampler");
        }

        Texture::~Texture()
        {
            vkDestroySampler(image_.get_device(), sampler_, nullptr);
        }
    }
}