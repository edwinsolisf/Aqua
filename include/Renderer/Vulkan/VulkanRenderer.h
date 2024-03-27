#pragma once 

#include "Core/Core.h"
#include "Window/Window.h"

#include "VulkanCore.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanDevice.h"

namespace Aqua
{
    namespace Vulkan
    {
        class Renderer
        {
        public:
            Renderer(Window& window);
            Renderer(const Renderer&) = delete;
            ~Renderer();

            bool is_valid() const noexcept { return successful_init_; }

            void draw_frame();
            void set_resize(bool resize) { framebuffer_resize_ = resize; }

            static bool Startup();
            static bool Shutdown();

            struct SwapChainSupportDetails
            {
                VkSurfaceCapabilitiesKHR capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR> present_modes;
            };

            struct ImageProperties
            {
                VkSurfaceFormatKHR format;
                VkExtent2D extent;
            };

        private:
            std::chrono::high_resolution_clock::time_point prev_time;
            std::chrono::high_resolution_clock::time_point curr_time;
            inline static std::vector<std::unique_ptr<UniformBuffer>> main_uniform_buffers;

            GLFWwindow* glfw_window_;

            std::vector<VkImage> swap_chain_images_;
            std::vector<VkImageView> swap_chain_image_views_;
            std::vector<VkFramebuffer> swap_chain_framebuffers_;
            VkSwapchainKHR swap_chain_;

            VkSurfaceKHR surface_;
            /*
            VkPhysicalDevice physical_device_;
            VkDevice logical_device_;
            */
            std::unique_ptr<Device> device_;

            static inline VkDescriptorSetLayout descriptor_set_layout_;
            VkDescriptorPool descriptor_pool_;
            inline static std::vector<VkDescriptorSet> descriptor_sets_;

            VkQueue graphics_queue_;
            VkQueue presents_queue_;
            
            ImageProperties image_properties_;
            inline static VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
            VkPipeline graphics_pipeline_;
            VkRenderPass render_pass_;
            bool successful_init_;

            VkCommandPool command_pool_;
            std::vector<VkCommandBuffer> command_buffers_;

            std::vector<VkSemaphore> image_available_semaphores_;
            std::vector<VkSemaphore> render_finished_semaphores_;
            std::vector<VkFence> in_flight_fences_;

            inline static std::unique_ptr<VertexBuffer> main_vertex_buffer = nullptr;
            inline static std::unique_ptr<IndexBuffer> main_index_buffer = nullptr;
            std::unique_ptr<Texture> main_texture = nullptr;

            inline static uint32_t current_frame_ = 0;
            bool framebuffer_resize_ = false;

            void recreate_swap_chain();
            void cleanup_swap_chain();

            static constexpr uint32_t max_frames_in_flight = 2;
            static VkInstance instance_;
            static std::vector<const char*> device_extensions_;
            static VkDebugUtilsMessengerEXT debug_messenger_;

            static std::vector<const char*> get_required_extensions();
            static bool check_required_extensions_availability(
                const std::vector<const char*>& required_extensions);

            static bool check_validation_layer_support();
            static VkDebugUtilsMessengerEXT create_debug_messenger();

            static VkSurfaceKHR create_window_surface(GLFWwindow* window);
            static VkSurfaceFormatKHR select_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
            static VkPresentModeKHR select_present_mode(const std::vector<VkPresentModeKHR>& available_modes);
            static VkExtent2D select_surface_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

            static VkPhysicalDevice select_physical_device(VkSurfaceKHR surface);
            static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
            static bool check_device_extension_support(VkPhysicalDevice device);

            static SwapChainSupportDetails get_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
            static std::pair<VkSwapchainKHR, ImageProperties> create_swap_chain(
                const Device& device,
                VkSurfaceKHR surface,
                GLFWwindow* window);

            static std::vector<VkImage> create_swap_chain_images(const Device& device, VkSwapchainKHR swap_chain);
            static std::vector<VkImageView> create_image_views(
                const Device& device,
                const std::vector<VkImage>& images,
                const ImageProperties&);

            static VkPipeline create_graphics_pipeline(
                VkDevice device,
                VkRenderPass render_pass,
                VkPipelineLayout pipeline_layout,
                const ImageProperties& image);

            static VkPipelineLayout create_graphics_pipeline_layout(VkDevice device);
            static VkShaderModule create_shader_module(VkDevice device, const std::vector<uint32_t>& code);
            static VkRenderPass create_render_pass(VkDevice device, const ImageProperties& image);

            static std::vector<VkFramebuffer> create_framebuffers(
                const Device& device,
                VkRenderPass render_pass,
                const std::vector<VkImageView>& image_views,
                const ImageProperties& properties);

            static void record_command_buffer(
                VkCommandBuffer buffer, 
                VkPipeline graphics_pipeline,
                VkRenderPass render_pass,
                const std::vector<VkFramebuffer>& framebuffers,
                uint32_t image_index,
                ImageProperties properties);

            static VkSemaphore create_semaphore(VkDevice device);
            static VkFence create_fence(VkDevice device);
            static VkDescriptorSetLayout create_descriptor_set_layout(VkDevice device);
            static VkDescriptorPool create_descriptor_pool(VkDevice device);

            static void bind_vertex_buffer(const VertexBuffer& vertex_buffer, VkCommandBuffer command_buffer);
        };
    }
}