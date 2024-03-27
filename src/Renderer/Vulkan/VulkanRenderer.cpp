#include "Renderer/Vulkan/VulkanRenderer.h"
#include "Renderer/Vulkan/VulkanDebug.h"

#include "Window/Window.h"
#include "Window/WindowInternal.h"

#include "Application/Application.h"

#include "Utils/ShaderCompilation.h"

#include "Debug/Debug.h"

#include "Math/stm/vector.h"
#include "Math/stm/spatial_transform.h"

#include <unordered_set>
#include <algorithm>

namespace Aqua
{
    namespace Vulkan
    {
        VkInstance Renderer::instance_ = nullptr;
        std::vector<const char*> Renderer::device_extensions_ = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            // VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
            // VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            // VK_EXT_PRIVATE_DATA_EXTENSION_NAME,
            VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME // Removes weird memory leak
        };
        VkDebugUtilsMessengerEXT Renderer::debug_messenger_ = VK_NULL_HANDLE;

        bool Renderer::Startup()
        {
            VkApplicationInfo app_info{};
            app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app_info.pApplicationName = "";
            app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.pEngineName = "VulkanRenderer";
            app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            app_info.apiVersion = VK_API_VERSION_1_3;

            VkInstanceCreateInfo instance_info{};
            instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instance_info.enabledLayerCount = 0;
            instance_info.pApplicationInfo = &app_info;
            
            auto required_extensions = get_required_extensions();
            
            #ifdef AQUA_MACOS_PLATFORM
                required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            #endif
            
            if (::Aqua::Vulkan::ENABLE_VALIDATION_LAYERS)
            {
                instance_info.enabledLayerCount = static_cast<uint32_t>(::Aqua::Vulkan::VALIDATION_LAYERS.size());
                instance_info.ppEnabledLayerNames = ::Aqua::Vulkan::VALIDATION_LAYERS.data();
            }

            if (!check_required_extensions_availability(required_extensions))
                AQUA_ERROR("Vulkan Error: missing required extensions for vulkan instance");

            instance_info.ppEnabledExtensionNames = required_extensions.data();
            instance_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());

            bool result = 
            vkCreateInstance(&instance_info, nullptr, &instance_) == VK_SUCCESS;

            debug_messenger_ = create_debug_messenger();
            if (::Aqua::Vulkan::ENABLE_VALIDATION_LAYERS && (debug_messenger_ == VK_NULL_HANDLE))
                AQUA_ERROR("Vulkan Error: failed to create vulkan debug messenger");

            return result;
        }

        bool Renderer::Shutdown()
        {
            if (::Aqua::Vulkan::ENABLE_VALIDATION_LAYERS)
                destroy_debug_utils_messenger_ext(instance_, debug_messenger_, nullptr);
            
            vkDestroyInstance(instance_, nullptr);

            return true;
        }

        Renderer::Renderer(Window& window)
            : surface_{ VK_NULL_HANDLE },
            successful_init_{ true }
        {
            glfw_window_ = window.get_internal_handle();

            surface_ = create_window_surface(glfw_window_);
            if (surface_ == VK_NULL_HANDLE)
            {
                AQUA_CRITICAL("Vulkan Error: failed to create window surface");
                successful_init_ = false;

                return;
            }

            auto physical_device = select_physical_device(surface_);
            if (physical_device == VK_NULL_HANDLE)
            {
                AQUA_CRITICAL("Vulkan Error: failed to find suitable GPU");
                successful_init_ = false;

                return;
            }

            device_ = std::make_unique<Device>(physical_device, surface_);

            auto logical_device = device_->get_device();
            if (logical_device == VK_NULL_HANDLE)
            {
                AQUA_CRITICAL("Vulkan Error: failed to create logical device");
                successful_init_ = false;
            }

            auto queue_families = device_->get_queue_families();
            graphics_queue_ = device_->get_graphics_queue();
            presents_queue_ = device_->get_present_queue();

            std::tie(swap_chain_, image_properties_) =
                create_swap_chain(*device_, surface_, glfw_window_);

            if (swap_chain_ == VK_NULL_HANDLE)
            {
                AQUA_CRITICAL("Vulkan Error: failed to create logical device");
                successful_init_ = false;
            }

            swap_chain_images_ = create_swap_chain_images(*device_, swap_chain_);
            swap_chain_image_views_ = create_image_views(*device_, swap_chain_images_, image_properties_);


            main_uniform_buffers.resize(max_frames_in_flight);
            for (auto& uniform : main_uniform_buffers)
                uniform = std::make_unique<UniformBuffer>(*device_, 0, UniformBufferObject{});
           
            main_texture = std::make_unique<Texture>(*device_, Application::get_assets_path() / "textures/final_kerr.png");

            // descriptor_set_layout_ = create_descriptor_set_layout(logical_device);
            descriptor_set_layout_ = [logical_device](){
                VkDescriptorSetLayout set_layout = VK_NULL_HANDLE;

                VkDescriptorSetLayoutBinding ubo_binding{};
                ubo_binding.binding = 0;
                ubo_binding.descriptorCount = 1;
                ubo_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                ubo_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                ubo_binding.pImmutableSamplers = nullptr;

                VkDescriptorSetLayoutBinding sampler_binding{};
                sampler_binding.binding = 1;
                sampler_binding.descriptorCount = 1;
                sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                sampler_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                sampler_binding.pImmutableSamplers = nullptr;

                std::array<VkDescriptorSetLayoutBinding, 2> bindings{ 
                    ubo_binding, sampler_binding
                };
                VkDescriptorSetLayoutCreateInfo info{};
                info.bindingCount = bindings.size();
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                info.pBindings = bindings.data();

                if (vkCreateDescriptorSetLayout(logical_device, &info, nullptr, &set_layout) != VK_SUCCESS)
                    AQUA_ERROR("Vulkan Error: failed to create descriptor set layout");

                return set_layout;
            }();

            // descriptor_pool_ = create_descriptor_pool(logical_device);
            descriptor_pool_ = [logical_device](){
                VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
                std::array<VkDescriptorPoolSize, 2> pool_sizes{};
                pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                pool_sizes[0].descriptorCount = static_cast<uint32_t>(max_frames_in_flight);
                pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
                pool_sizes[1].descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

                VkDescriptorPoolCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                info.poolSizeCount = pool_sizes.size();
                info.pPoolSizes = pool_sizes.data();
                info.maxSets = static_cast<uint32_t>(max_frames_in_flight);

                if (vkCreateDescriptorPool(logical_device, &info, nullptr, &descriptor_pool) != VK_SUCCESS)
                    AQUA_ERROR("Vulkan Error: failed to create descriptor pool");

                return descriptor_pool;
            }();
            descriptor_sets_.resize(max_frames_in_flight);
            
            std::vector<VkDescriptorSetLayout> layout(max_frames_in_flight, descriptor_set_layout_);
            VkDescriptorSetAllocateInfo set_allocate_info{};
            set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            set_allocate_info.descriptorPool = descriptor_pool_;
            set_allocate_info.descriptorSetCount = static_cast<uint32_t>(max_frames_in_flight);
            set_allocate_info.pSetLayouts = layout.data();

            if (vkAllocateDescriptorSets(logical_device, &set_allocate_info, descriptor_sets_.data()) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to allocate descriptor sets");

            for (size_t i = 0; i < max_frames_in_flight; ++i)
            {
                VkDescriptorBufferInfo buffer_info{};
                buffer_info.buffer = main_uniform_buffers[i]->get_buffer();
                buffer_info.offset = 0;
                buffer_info.range = main_uniform_buffers[i]->get_buffer_size();

                VkDescriptorImageInfo image_info{};
                image_info.imageLayout = main_texture->get_image().get_layout();
                image_info.imageView = main_texture->get_image().get_view();
                image_info.sampler = main_texture->get_sampler();

                std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
                descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_writes[0].dstSet = descriptor_sets_[i];
                descriptor_writes[0].dstBinding = 0;
                descriptor_writes[0].dstArrayElement = 0;
                descriptor_writes[0].descriptorCount = 1;
                descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptor_writes[0].pBufferInfo = &buffer_info;
                descriptor_writes[0].pImageInfo = nullptr;
                descriptor_writes[0].pTexelBufferView = nullptr;

                descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptor_writes[1].dstSet = descriptor_sets_[i];
                descriptor_writes[1].dstBinding = 1;
                descriptor_writes[1].dstArrayElement = 0;
                descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptor_writes[1].descriptorCount = 1;
                descriptor_writes[1].pImageInfo = &image_info;

                vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(descriptor_writes.size()),
                    descriptor_writes.data(), 0, nullptr);
            }

            pipeline_layout_ = create_graphics_pipeline_layout(logical_device);
            render_pass_ = create_render_pass(logical_device, image_properties_);
            graphics_pipeline_ = create_graphics_pipeline(logical_device, render_pass_, pipeline_layout_, image_properties_);
            swap_chain_framebuffers_ = create_framebuffers(*device_, render_pass_, swap_chain_image_views_, image_properties_);

            command_pool_ = device_->create_command_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

            // record_command_buffer(command_buffer_, graphics_pipeline_, render_pass_, swap_chain_framebuffers_, 0, image_properties_);
            command_buffers_.resize(max_frames_in_flight);
            in_flight_fences_.resize(max_frames_in_flight);
            image_available_semaphores_.resize(max_frames_in_flight);
            render_finished_semaphores_.resize(max_frames_in_flight);

            for (auto& command_buffer : command_buffers_)
                command_buffer = device_->create_command_buffer(command_pool_);

            for (auto& semaphore : image_available_semaphores_)
                semaphore = create_semaphore(logical_device);

            for (auto& semaphore : render_finished_semaphores_)
                semaphore = create_semaphore(logical_device);

            for (auto& fence : in_flight_fences_)
                fence = create_fence(logical_device);

            main_vertex_buffer = std::make_unique<VertexBuffer>(
                *device_,
                std::vector<Vertex> {
                    { {-0.5,-0.5} , {1.0, 0.0, 0.0} , { 1.f, 0.f}},
                    { {-0.5, 0.5} , {0.0, 1.0, 0.0} , { 0.f, 0.f}},
                    { { 0.5, 0.5} , {0.0, 0.0, 1.0} , { 0.f, 1.f}}
                    ,
                    // { { 0.5, 0.5} , {0.0, 0.0, 1.0} },
                    { { 0.5,-0.5} , {0.0, 1.0, 0.0} , {1.f, 1.f}}
                    // ,
                    // { {-0.5,-0.5} , {1.0, 0.0, 0.0} }
            
                    // { {0.5, -0.5} , {1.0, 0.0, 0.0} },
                    // { {0.5, 0.5} , {0.0, 1.0, 0.0} },
                    // { {-0.5, 0.5} , {0.0, 1.0, 1.0} }
                }
            );

            main_index_buffer = std::make_unique<IndexBuffer>(
                *device_,
                std::vector<uint32_t>{
                    0, 1, 2, 2, 3, 0
                }
            );
        }

        Renderer::~Renderer()
        {
            device_->wait_idle();

            main_vertex_buffer = nullptr;
            main_index_buffer = nullptr;

            auto logical_device = device_->get_device();


            for (auto semaphore : image_available_semaphores_)
                vkDestroySemaphore(logical_device, semaphore, nullptr);

            for (auto semaphore : render_finished_semaphores_)
                vkDestroySemaphore(logical_device, semaphore, nullptr);

            for (auto fence: in_flight_fences_)
                vkDestroyFence(logical_device, fence, nullptr);

            cleanup_swap_chain();

            vkDestroyDescriptorPool(logical_device, descriptor_pool_, nullptr);
            vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout_, nullptr);

            vkFreeCommandBuffers(logical_device, command_pool_, command_buffers_.size(), command_buffers_.data());
            vkDestroyCommandPool(logical_device, command_pool_, nullptr);

            // for (const auto& image : swap_chain_images_)
            //     vkDestroyImage(logical_device, image, nullptr);

            vkDestroyPipeline(logical_device, graphics_pipeline_, nullptr);
            vkDestroyRenderPass(logical_device, render_pass_, nullptr);
            vkDestroyPipelineLayout(logical_device, pipeline_layout_, nullptr);

            device_ = nullptr;

            vkDestroySurfaceKHR(instance_, surface_, nullptr);
        }

        void Renderer::draw_frame()
        {
            const auto& window = Application::get().get_window();

            UniformBufferObject ubo{};
            auto time = std::chrono::duration<float, std::chrono::seconds::period>(curr_time - prev_time);
            // AQUA_INFO(std::string("FPS: ") + std::to_string(1.f/time.count()));
            static float angle = 0.f;
            angle += time.count() * std::numbers::pi / 2.0;
            angle = fmod(angle, std::numbers::pi * 2);
            auto rot = stm::rotate<float>({0.f, 0.f, 1.f}, angle);
            ubo.model = rot.transpose();
            constexpr auto global_up = stm::vector{ 0.f, 1.f, 0.f };
            constexpr auto look = stm::vector{ 0.f, 0.f, 0.f }; 
            constexpr auto pos = stm::vector{ 2.f, 2.f , 2.f };
            constexpr auto dir = (look - pos).unit();
            constexpr auto right = stm::cross(dir, global_up).unit();
            constexpr auto up = stm::cross(right, dir).unit();
            constexpr auto view = stm::lookAt<float>(pos, up, right);
            ubo.view = view.transpose();
            ubo.projection = stm::perspective<float>(std::numbers::pi / 2, (float)window.get_width() / (float)window.get_height(), 0.1, 10.).transpose();

            main_uniform_buffers[current_frame_]->set_uniform_data(ubo);

            vkWaitForFences(device_->get_device(), 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

            uint32_t image_index = 0;
            auto acquire_result = vkAcquireNextImageKHR(device_->get_device(),
                                                        swap_chain_,
                                                        UINT64_MAX,
                                                        image_available_semaphores_[current_frame_],
                                                        VK_NULL_HANDLE,
                                                        &image_index);

            if (framebuffer_resize_ || acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                AQUA_INFO("Vulkan Info: Recreated swap chain");
                framebuffer_resize_ = false;

                recreate_swap_chain();
                return;
            }
            else if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
            {
                AQUA_ERROR("Vulkan Error: failed to acquire swap chain image");
            }

            vkResetFences(device_->get_device(), 1, &in_flight_fences_[current_frame_]);

            vkResetCommandBuffer(command_buffers_[current_frame_], 0);
            record_command_buffer(command_buffers_[current_frame_],
                                graphics_pipeline_,
                                render_pass_,
                                swap_chain_framebuffers_,
                                image_index,
                                image_properties_);


            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore wait_semaphores[] = { image_available_semaphores_[current_frame_] };

            VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = wait_semaphores;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffers_[current_frame_];
            submit_info.pWaitDstStageMask = wait_stages;

            VkSemaphore signal_semaphores[] = { render_finished_semaphores_[current_frame_] };
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = signal_semaphores;

            if (vkQueueSubmit(graphics_queue_, 1, &submit_info, in_flight_fences_[current_frame_]) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to submit draw command buffer");

            VkPresentInfoKHR present_info{};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = signal_semaphores;
            
            VkSwapchainKHR swap_chains[] = { swap_chain_ };
            present_info.swapchainCount = 1;
            present_info.pSwapchains = swap_chains;
            present_info.pImageIndices = &image_index;
            present_info.pResults = nullptr;

            auto result = vkQueuePresentKHR(presents_queue_, &present_info);
            if (framebuffer_resize_ || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                AQUA_INFO("Vulkan Info: Recreated swap chain");
                recreate_swap_chain();

                framebuffer_resize_ = false;
            }
            else if (result != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to queue presentation of image");
            
            current_frame_ = (current_frame_ + 1) % max_frames_in_flight;

            if (::Aqua::Vulkan::ENABLE_VALIDATION_LAYERS)
                device_->wait_idle();

            prev_time = curr_time;
            curr_time = std::chrono::high_resolution_clock::now();
        }

        void Renderer::recreate_swap_chain()
        {
            device_->wait_idle();

            cleanup_swap_chain();

            std::tie(swap_chain_, image_properties_) = create_swap_chain(*device_, surface_, glfw_window_);
            swap_chain_images_ = create_swap_chain_images(*device_, swap_chain_);
            swap_chain_image_views_ = create_image_views(*device_, swap_chain_images_, image_properties_);
            swap_chain_framebuffers_ = create_framebuffers(*device_, render_pass_, swap_chain_image_views_, image_properties_);
            
            for (auto& semaphore : image_available_semaphores_)
                semaphore = create_semaphore(device_->get_device());
        }
        
        void Renderer::cleanup_swap_chain()
        {
            auto logical_device = device_->get_device();
        
            for (auto semaphore : image_available_semaphores_)
                vkDestroySemaphore(logical_device, semaphore, nullptr);

            for (auto framebuffer : swap_chain_framebuffers_)
                vkDestroyFramebuffer(logical_device, framebuffer, nullptr);

            for (const auto& view : swap_chain_image_views_)
                vkDestroyImageView(logical_device, view, nullptr);

            vkDestroySwapchainKHR(logical_device, swap_chain_, nullptr);
        }

        std::vector<const char*> Renderer::get_required_extensions()
        {
            std::vector<const char*> extensions;

            uint32_t glfw_extension_count = 0;
            const char** glfw_extensions = nullptr;

            glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            extensions.reserve(glfw_extension_count + 2);
            extensions.resize(glfw_extension_count);
            std::copy(glfw_extensions, glfw_extensions + glfw_extension_count, extensions.data());

            if (::Aqua::Vulkan::ENABLE_VALIDATION_LAYERS)
                extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            return extensions;
        }

        bool Renderer::check_required_extensions_availability
        (const std::vector<const char*>& required_extensions)
        {
            uint32_t extension_count = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> extensions(extension_count);
            vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

            std::unordered_set<std::string> extension_set;
            for (const auto& extension : extensions)
                extension_set.emplace(extension.extensionName);

            std::string current_extension;
            for (const auto& extension : required_extensions)
            {
                current_extension = extension;
                if (!extension_set.contains(current_extension))
                {
                    AQUA_WARN(std::string("Vulkan Warning: missing extension ") + current_extension);
                    return false;
                }
            }

            return true;
        }

        bool Renderer::check_validation_layer_support()
        {
            uint32_t layer_count = 0;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            std::vector<VkLayerProperties> available_layers(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
            
            for (const auto& layer_name : ::Aqua::Vulkan::VALIDATION_LAYERS)
            {
                bool layer_found = false;

                for (const auto& layer_properties : available_layers)
                {
                    if (strcmp(layer_name, layer_properties.layerName) == 0)
                    {
                        layer_found = true;
                        break;
                    }
                }

                if (!layer_found)
                    return false;
            }

            return true;
        }

        VkDebugUtilsMessengerEXT Renderer::create_debug_messenger()
        {
            VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

            VkDebugUtilsMessengerCreateInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            info.pfnUserCallback = debug_callback;
            info.pUserData = nullptr;

            if (create_debug_utils_messenger_ext(instance_, &info, nullptr, &messenger) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create debug messenger");

            return messenger;
        }

        VkSurfaceKHR Renderer::create_window_surface(GLFWwindow* window)
        {
            VkSurfaceKHR surface;

            glfwCreateWindowSurface(instance_, window, nullptr, &surface);

            return surface;
        }

        VkSurfaceFormatKHR Renderer::select_surface_format(
            const std::vector<VkSurfaceFormatKHR>& available_formats)
        {
            for (const auto& available_format : available_formats)
            {
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return available_format;
            }

            return available_formats.front();
        }

        VkPresentModeKHR Renderer::select_present_mode(const std::vector<VkPresentModeKHR>& available_modes)
        {
            for (const auto& mode : available_modes)
            {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return mode;
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D Renderer::select_surface_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
                return capabilities.currentExtent;
            else
            {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);

                VkExtent2D actual_extent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actual_extent.width = std::clamp(actual_extent.width,
                    capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

                actual_extent.height = std::clamp(actual_extent.height,
                    capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actual_extent;
            }
        }

        VkPhysicalDevice Renderer::select_physical_device(VkSurfaceKHR surface)
        {
            VkPhysicalDevice physical_device = VK_NULL_HANDLE;
            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(instance_,  &device_count, nullptr);

            std::vector<VkPhysicalDevice> devices(device_count);
            vkEnumeratePhysicalDevices(instance_,  &device_count, devices.data());

            for (const auto& device : devices)
            {
                if (is_device_suitable(device, surface))
                {
                    physical_device = device;
                    break;
                }    
            }

            return physical_device;
        }

        bool Renderer::is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(device, &device_properties);
            
            VkPhysicalDeviceFeatures device_features;
            vkGetPhysicalDeviceFeatures(device, &device_features);

            auto queue_families = Device::find_queue_families(device, surface);

            auto is_gpu = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto has_features = device_features.geometryShader;
            auto extensions_supported = check_device_extension_support(device);
            auto swap_chain_adequate = false;

            if (extensions_supported)
            {
                SwapChainSupportDetails details = get_swap_chain_support(device, surface);
                swap_chain_adequate = !details.formats.empty() && !details.present_modes.empty();
            }

            return has_features && queue_families.is_graphics_complete() && swap_chain_adequate && is_gpu;
        }

        bool Renderer::check_device_extension_support(VkPhysicalDevice device)
        {
            uint32_t extension_count = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

            std::unordered_set<std::string> extension_set;
            for (const auto& extension : extensions)
                extension_set.emplace(extension.extensionName);

            std::string current_extension;
            for (const auto& extension : device_extensions_)
            {
                current_extension = extension;
                if (!extension_set.contains(current_extension))
                {
                    AQUA_WARN(std::string("Vulkan Warning: missing extension ") + current_extension);
                    return false;
                }
            }

            return true;
        }

        Renderer::SwapChainSupportDetails Renderer::get_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t format_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());

            uint32_t present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());

            return details;
        }

        std::pair<VkSwapchainKHR, Renderer::ImageProperties> Renderer::create_swap_chain(
            const Device& device,
            VkSurfaceKHR surface,
            GLFWwindow* window)
        {
            SwapChainSupportDetails swap_chain_support = get_swap_chain_support(device.get_physical_device(), surface);

            VkSurfaceFormatKHR surface_format = select_surface_format(swap_chain_support.formats);
            VkPresentModeKHR present_mode = select_present_mode(swap_chain_support.present_modes);
            VkExtent2D extent = select_surface_extent(swap_chain_support.capabilities, window);
            uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
            
            if (swap_chain_support.capabilities.maxImageCount > 0 &&
                image_count > swap_chain_support.capabilities.maxImageCount)
                image_count = swap_chain_support.capabilities.maxImageCount;

            VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
            VkSwapchainCreateInfoKHR info{};
            info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            info.surface = surface;
            info.minImageCount = image_count;
            info.imageColorSpace = surface_format.colorSpace;
            info.imageFormat = surface_format.format;
            info.imageExtent = extent;
            info.imageArrayLayers = 1;
            info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            const auto& indices = device.get_queue_families();
            uint32_t queue_family_indices[] = { indices.graphics_family.value() , indices.present_family.value() };

            if (indices.graphics_family != indices.present_family)
            {
                info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                info.queueFamilyIndexCount = 2;
                info.pQueueFamilyIndices = queue_family_indices;
            }
            else
            {
                info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                info.queueFamilyIndexCount = 0;
                info.pQueueFamilyIndices = nullptr;
            }

            info.preTransform = swap_chain_support.capabilities.currentTransform;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.presentMode = present_mode;
            info.clipped = VK_TRUE;
            info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            info.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(device.get_device(), &info, nullptr, &swap_chain) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create surface swap chain");

            return { swap_chain, { surface_format , extent } };
        }

        std::vector<VkImage> Renderer::create_swap_chain_images(const Device& device, VkSwapchainKHR swap_chain)
        {
            uint32_t image_count = 0;
            vkGetSwapchainImagesKHR(device.get_device(), swap_chain, &image_count, nullptr);

            std::vector<VkImage> images(image_count);
            vkGetSwapchainImagesKHR(device.get_device(), swap_chain, &image_count, images.data());

            return images;
        }

        std::vector<VkImageView> Renderer::create_image_views(
            const Device& device,
            const std::vector<VkImage>& images,
            const ImageProperties& properties)
        {
            std::vector<VkImageView> views(images.size());

            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = properties.format.format;
            info.components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            };
            info.subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            for (uint32_t i = 0; i < views.size(); ++i)
            {
                auto image_info = info;
                image_info.image = images[i];
                if (vkCreateImageView(device.get_device(), &image_info, nullptr, &views[i]) != VK_SUCCESS)
                    AQUA_ERROR("Vulkan Error: failed to create image view");
            }

            return views;
        }

        VkShaderModule Renderer::create_shader_module(VkDevice device, const std::vector<uint32_t>& code)
        {
            VkShaderModule shader_module = VK_NULL_HANDLE;

            VkShaderModuleCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            info.codeSize = code.size() * sizeof(uint32_t);
            info.pCode = code.data();

            if (vkCreateShaderModule(device, &info, nullptr, &shader_module) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create shader module");

            return shader_module;
        }

        VkPipelineLayout Renderer::create_graphics_pipeline_layout(VkDevice device)
        {
            VkPipelineLayout pipeline_layout;

            VkPipelineLayoutCreateInfo layout_info{};
            layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            layout_info.setLayoutCount = 1;
            layout_info.pSetLayouts = &descriptor_set_layout_;
            layout_info.pushConstantRangeCount = 0;
            layout_info.pPushConstantRanges = nullptr;

            if (vkCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create pipeline layout");

            return pipeline_layout;
        }

        VkPipeline Renderer::create_graphics_pipeline(
            VkDevice device,
            VkRenderPass render_pass,
            VkPipelineLayout pipeline_layout,
            const ImageProperties& image)
        {
            VkPipeline pipeline = VK_NULL_HANDLE;

            auto vert_shader_code = create_shader_module(device, compile_shader_from_file(
                                                        Application::get_assets_path() / "shaders/vertex.vert.glsl"));
            auto frag_shader_code = create_shader_module(device, compile_shader_from_file(
                                                        Application::get_assets_path() / "shaders/vertex.frag.glsl"));

            VkPipelineShaderStageCreateInfo vert_info{};
            vert_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_info.module = vert_shader_code;
            vert_info.pName = "main";

            VkPipelineShaderStageCreateInfo frag_info{};
            frag_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_info.module = frag_shader_code;
            frag_info.pName = "main";

            VkPipelineShaderStageCreateInfo shader_stages[] = {
                vert_info, frag_info
            };

            // VkPipelineVertexInputStateCreateInfo vertex_input_info{};
            // vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            // vertex_input_info.vertexAttributeDescriptionCount = 0;
            // vertex_input_info.pVertexAttributeDescriptions = nullptr;
            // vertex_input_info.vertexBindingDescriptionCount = 0;
            // vertex_input_info.pVertexBindingDescriptions = nullptr;
            auto vertex_input_info = Vertex::create_vertex_input_info();

            VkPipelineInputAssemblyStateCreateInfo input_assembly{};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly.primitiveRestartEnable = VK_FALSE;

            std::vector<VkDynamicState> dynamic_states = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamic_state{};
            dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
            dynamic_state.pDynamicStates = dynamic_states.data();

            // VkViewport viewport{};
            // viewport.x = 0.f;
            // viewport.y = 0.f;
            // viewport.width = image.extent.width;
            // viewport.height = image.extent.height;
            // viewport.minDepth = 0.f;
            // viewport.maxDepth = 1.f;

            // VkRect2D scissor{};
            // scissor.offset = { 0 , 0 };
            // scissor.extent = image.extent;

            VkPipelineViewportStateCreateInfo viewport_state{};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.scissorCount = 1;
            viewport_state.viewportCount = 1;
            // viewport_state.pScissors = &scissor;
            // viewport_state.pViewports = &viewport;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            // rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

            rasterizer.depthBiasEnable = VK_FALSE;
            rasterizer.depthBiasConstantFactor = 0.0f;
            rasterizer.depthBiasClamp = 0.0f;
            rasterizer.depthBiasSlopeFactor = 0.0f;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampling.minSampleShading = 1.0f;
            multisampling.pSampleMask = nullptr;
            multisampling.alphaToCoverageEnable = VK_FALSE;
            multisampling.alphaToOneEnable = VK_FALSE;

            VkPipelineColorBlendAttachmentState color_blend_attachment{};
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                    VK_COLOR_COMPONENT_G_BIT |
                                                    VK_COLOR_COMPONENT_B_BIT |
                                                    VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment.blendEnable = VK_FALSE;
            color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blending{};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.logicOpEnable = VK_FALSE;
            color_blending.logicOp = VK_LOGIC_OP_COPY;
            color_blending.attachmentCount = 1;
            color_blending.pAttachments = &color_blend_attachment;
            color_blending.blendConstants[0] = 0.0f;
            color_blending.blendConstants[1] = 0.0f;
            color_blending.blendConstants[2] = 0.0f;
            color_blending.blendConstants[3] = 0.0f;

            VkGraphicsPipelineCreateInfo pipeline_info{};
            pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_info.stageCount = 2;
            pipeline_info.pStages = shader_stages;
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer;
            pipeline_info.pMultisampleState = &multisampling;
            pipeline_info.pDepthStencilState = nullptr;
            pipeline_info.pColorBlendState = &color_blending;
            pipeline_info.pDynamicState = &dynamic_state;
            pipeline_info.layout = pipeline_layout;
            pipeline_info.renderPass = render_pass;
            pipeline_info.subpass = 0;
            pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
            pipeline_info.basePipelineIndex = -1;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create graphics pipeline");

            vkDestroyShaderModule(device, vert_shader_code, nullptr);
            vkDestroyShaderModule(device, frag_shader_code, nullptr);

            return pipeline;
        }

        VkRenderPass Renderer::create_render_pass(VkDevice device, const ImageProperties& image)
        {
            VkRenderPass render_pass = VK_NULL_HANDLE;

            VkAttachmentDescription color_attachment{};
            color_attachment.format = image.format.format;
            color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference color_attachment_ref{};
            color_attachment_ref.attachment = 0;
            color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_attachment_ref;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_info.attachmentCount = 1;
            render_pass_info.pAttachments = &color_attachment;
            render_pass_info.subpassCount = 1;
            render_pass_info.pSubpasses = &subpass;

            render_pass_info.dependencyCount = 1;
            render_pass_info.pDependencies = &dependency;

            if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass))
                AQUA_ERROR("Vulkan Error: failed to create render pass");

            return render_pass;
        }

        std::vector<VkFramebuffer> Renderer::create_framebuffers(
                const Device& device,
                VkRenderPass render_pass,
                const std::vector<VkImageView>& image_views,
                const ImageProperties& properties)
        {
            std::vector<VkFramebuffer> framebuffers(image_views.size(), VK_NULL_HANDLE);

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = render_pass;
            framebuffer_info.width = properties.extent.width;
            framebuffer_info.height = properties.extent.height;
            framebuffer_info.layers = 1;

            for (size_t i = 0; i < image_views.size(); ++i)
            {
                VkImageView attachments[] = {
                    image_views[i]
                };

                framebuffer_info.attachmentCount = 1;
                framebuffer_info.pAttachments = attachments;

                if (vkCreateFramebuffer(device.get_device(), &framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS)
                    AQUA_ERROR("Vulkan Error: failed to create framebuffer");
            }

            return framebuffers;
        }

        void Renderer::record_command_buffer(
            VkCommandBuffer buffer, 
            VkPipeline graphics_pipeline,
            VkRenderPass render_pass,
            const std::vector<VkFramebuffer>& framebuffers,
            uint32_t image_index,
            ImageProperties properties)
        {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags = 0;

            if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS)
            {
                AQUA_ERROR("Vulkan Error: failed to begin recording command buffer");
                return;
            }

            VkRenderPassBeginInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.framebuffer = framebuffers[image_index];
            render_pass_info.renderArea.extent = properties.extent;
            render_pass_info.renderArea.offset = { 0 , 0 };
            render_pass_info.renderPass = render_pass;

            VkClearValue clear_color{{{0.f, 0.f, 0.f, 0.f}}};
            render_pass_info.pClearValues = &clear_color;
            render_pass_info.clearValueCount = 1;

            vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

                VkViewport viewport{};
                viewport.x = 0.f;
                viewport.y = 0.f;
                viewport.width = properties.extent.width;
                viewport.height = properties.extent.height;
                viewport.minDepth = 0.f;
                viewport.maxDepth = 1.f;

                VkRect2D scissor{};
                scissor.offset = { 0 , 0 };
                scissor.extent = properties.extent;

                vkCmdSetViewport(buffer, 0, 1, &viewport);
                vkCmdSetScissor(buffer, 0, 1, &scissor);

                // bind_vertex_buffer(*main_vertex_buffer, buffer);
                // vkCmdDraw(buffer, main_vertex_buffer->get_vertex_count(), 1, 0, 0);            

                main_vertex_buffer->bind_buffer(buffer);
                main_index_buffer->bind_buffer(buffer);
                vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
                    &descriptor_sets_[current_frame_], 0, nullptr);

                vkCmdDrawIndexed(buffer, main_index_buffer->get_index_count(), 1, 0, 0, 0);
            }
            vkCmdEndRenderPass(buffer);

            if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to record command buffer");
        }

        void Renderer::bind_vertex_buffer(const VertexBuffer& vertex_buffer, VkCommandBuffer command_buffer)
        {
            VkBuffer vertex_buffers[] = { vertex_buffer.get_buffer() };
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        }

        VkSemaphore Renderer::create_semaphore(VkDevice device)
        {
            VkSemaphore semaphore = VK_NULL_HANDLE;

            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(device, &info, nullptr, &semaphore) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create semaphore");

            return semaphore;
        }

        VkFence Renderer::create_fence(VkDevice device)
        {
            VkFence fence = VK_NULL_HANDLE;

            VkFenceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if (vkCreateFence(device, &info, nullptr, &fence) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create fence");

            return fence;
        }

        VkDescriptorSetLayout Renderer::create_descriptor_set_layout(VkDevice device)
        {
            VkDescriptorSetLayout layout = VK_NULL_HANDLE;

            VkDescriptorSetLayoutBinding ubo_layout_binding{};
            ubo_layout_binding.binding = 0;
            ubo_layout_binding.descriptorCount = 1;
            ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            ubo_layout_binding.pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo descriptor_info{};
            descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_info.bindingCount = 1;
            descriptor_info.pBindings = &ubo_layout_binding;

            if (vkCreateDescriptorSetLayout(device, &descriptor_info, nullptr, &layout) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create descriptor set");

            return layout;
        }

        VkDescriptorPool Renderer::create_descriptor_pool(VkDevice device)
        {
            VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

            VkDescriptorPoolSize pool_size{};
            pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            pool_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

            VkDescriptorPoolCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            info.poolSizeCount = 1;
            info.pPoolSizes = &pool_size;
            info.maxSets = static_cast<uint32_t>(max_frames_in_flight);

            if (vkCreateDescriptorPool(device, &info, nullptr, &descriptor_pool) != VK_SUCCESS)
                AQUA_ERROR("Vulkan Error: failed to create descriptor pool");

            return descriptor_pool;
        }
    }
}