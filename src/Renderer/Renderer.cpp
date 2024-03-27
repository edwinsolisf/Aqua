#include "Renderer/Renderer.h"

#include "Renderer/Vulkan/VulkanRenderer.h"

namespace Aqua
{
    Renderer::Renderer(Window& window, std::shared_ptr<EventQueue> queue)
        : handle_{ std::make_unique<Vulkan::Renderer>(window) } , queue_ { queue }
    {
    }
    
    Renderer::~Renderer() = default;

    bool Renderer::Startup()
    {
        return Vulkan::Renderer::Startup();
    }

    bool Renderer::Shutdown()
    {
        return Vulkan::Renderer::Shutdown();
    }

    bool Renderer::handle_events() const
    {
        return queue_->handle([this](const Event& event){
            if (event.get_type() == Event::Types::Window_Resized)
            {
                this->handle_->set_resize(true);
                return true;
            }
            return false;
        });
    }

    void Renderer::render() const { handle_->draw_frame(); }

    bool Renderer::is_valid() const noexcept
    {
        return handle_->is_valid();
    }
}