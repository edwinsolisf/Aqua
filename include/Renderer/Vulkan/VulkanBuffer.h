#pragma once

#include "Core/Core.h"
#include "Debug/Debug.h"
#include "Math/stm/vector2.h"
#include "Math/stm/vector3.h"
#include "Math/stm/spatial_transform.h"

#include "VulkanCore.h"
#include "VulkanDevice.h"
#include "VulkanBufferBase.h"

namespace Aqua
{
    namespace Vulkan
    {
        struct Vertex
        {
            using vec3f = stm::vector<float, 3>;
            using vec2f = stm::vector<float, 2>;

            vec2f position;
            vec3f color;
            vec2f text_coord;

            static VkVertexInputBindingDescription get_binding_description()
            {
                VkVertexInputBindingDescription description{};
                description.binding = 0;
                description.stride = sizeof(Vertex);
                description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return description;
            }

            static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions()
            {
                std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};
                
                attribute_descriptions[0].binding = 0;
                attribute_descriptions[0].location = 0;
                attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
                attribute_descriptions[0].offset = offsetof(Vertex, position);

                attribute_descriptions[1].binding = 0;
                attribute_descriptions[1].location = 1;
                attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attribute_descriptions[1].offset = offsetof(Vertex, color);

                attribute_descriptions[2].binding = 0;
                attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                attribute_descriptions[2].location = 2;
                attribute_descriptions[2].offset = offsetof(Vertex, text_coord);

                return attribute_descriptions;
            }

            static VkPipelineVertexInputStateCreateInfo create_vertex_input_info()
            {
                static VkPipelineVertexInputStateCreateInfo info{};
                
                static auto binding_description = get_binding_description();
                static auto attribute_description = get_attribute_descriptions();

                info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                info.vertexBindingDescriptionCount = 1;
                info.pVertexBindingDescriptions = &binding_description;
                info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
                info.pVertexAttributeDescriptions = attribute_description.data();

                return info;
            }
        };

        class VertexBuffer : public Buffer
        {
        public:
            template<typename T>
            VertexBuffer(const Device& device, const std::vector<T>& vertices)
                : Buffer{ device.create_buffer(vertices.size() * sizeof(T),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) },
                  vertex_count_{ static_cast<uint32_t>(vertices.size()) }
            {
                create_vertex_buffer(device, vertices.data(), vertices.size() * sizeof(T));
            }

            VertexBuffer(const VertexBuffer&) = delete;

            uint32_t get_vertex_count() const noexcept { return vertex_count_; }

            void bind_buffer(VkCommandBuffer command_buffer) const;

        private:
            uint32_t vertex_count_ = 0;

            void create_vertex_buffer(const Device& device, const void* data, VkDeviceSize size);
        };

        class IndexBuffer : public Buffer
        {
        public:
            IndexBuffer(const Device& device, const std::vector<uint32_t>& indices);

            IndexBuffer(const VertexBuffer&) = delete;

            uint32_t get_index_count() const noexcept { return index_count_; }

            void bind_buffer(VkCommandBuffer command_buffer) const;

        private:
            uint32_t index_count_ = 0;
        };

        void copy_device_buffers(const Device& device,
                                    VkBuffer src_buffer,
                                    VkBuffer dst_buffer,
                                    VkDeviceSize size,
                                    VkDeviceSize src_offset = 0,
                                    VkDeviceSize dst_offset = 0);

        struct UniformBufferObject
        {
            stm::mat4f model;
            stm::mat4f view;
            stm::mat4f projection;
        };

        class UniformBuffer : public Buffer
        {
        public:
            template<typename T>
            requires std::is_pointer_v<T>
            UniformBuffer(const Device& device, uint32_t binding, const T uniform_data)
                : Buffer{ device.create_buffer(sizeof(std::remove_pointer_t<T>),
                                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) },
                  mapped_memory_{ nullptr },
                  binding_{ binding }
            {
                create_uniform_buffer(device, uniform_data);
            }

            template<typename T>
            requires !std::is_pointer_v<T>
            UniformBuffer(const Device& device, uint32_t binding, const T& uniform_data)
                : UniformBuffer(device, binding, &uniform_data) {}

            UniformBuffer(const UniformBuffer&) = delete;

            template<typename T>
            requires !std::is_pointer_v<T>
            void set_uniform_data(const T& data) const
            {
                set_uniform_data(&data);
            }

            template<typename T>
            requires std::is_pointer_v<T>
            void set_uniform_data(const T data) const
            {
                constexpr auto size = sizeof(std::remove_pointer_t<T>);
                AQUA_ASSERT(size == buffer_size_, "Initial and current type sizes must match");
                if (size != buffer_size_)
                    AQUA_ERROR("Vulkan Error: Uniform buffer size is not the same size as the size of the type passed");

                if (data != nullptr)                
                    std::copy((const uint8_t*)data, (const uint8_t*)data + buffer_size_, (uint8_t*)mapped_memory_);
                else
                    AQUA_ERROR("Vulkan Error: nullptr passed, no data copied to uniform buffer");
            }

            void bind_buffer(VkCommandBuffer command_buffer) const;

        private:
            uint32_t binding_;
            void* mapped_memory_;

            void create_uniform_buffer(const Device& device, const void* data);
        };
    }
}