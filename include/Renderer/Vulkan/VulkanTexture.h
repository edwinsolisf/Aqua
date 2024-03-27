#pragma once

#include "VulkanCore.h"
#include "VulkanDevice.h"

namespace Aqua
{
    namespace Vulkan
    {
        class Texture
        {
        public:
            Texture(const Device& device, const std::filesystem::path& filepath);
            Texture(const Texture&) = delete;
            ~Texture();

            const Image& get_image() const noexcept { return image_; }
            VkSampler get_sampler() const noexcept { return sampler_; }

        private:
            Image image_;
            VkSampler sampler_;
        };
    }
}