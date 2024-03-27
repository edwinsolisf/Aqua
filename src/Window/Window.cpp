#include "Window/window.h"
#include "Window/WindowInternal.h"

namespace Aqua
{
    Window::Window(std::shared_ptr<EventQueue> queue)
        : impl_{ std::make_unique<WindowImpl>(queue) } , queue_{ queue }
    {   
    }

    Window::Window(uint32_t width, uint32_t height, std::string_view title, std::shared_ptr<EventQueue> queue)
        : impl_{ std::make_unique<WindowImpl>(width, height, title, queue) } , queue_{ queue }
    {
    }

    Window::~Window() = default;

    uint32_t Window::get_width() const
    {
        return impl_->get_width();
    }

    uint32_t Window::get_height() const
    {
        return impl_->get_height();
    }

    bool Window::update() const
    {
        if (impl_->should_close()) [[unlikely]]
            return false;

        impl_->update();

        return true;
    }

    bool Window::handle_events() const
    {
        return false;
    }

    bool Window::Startup()
    {
        return WindowImpl::Startup();
    }

    bool Window::Shutdown()
    {
        return WindowImpl::Shutdown();
    }

    GLFWwindow* Window::get_internal_handle() const
    {
        return impl_->get_glfw_window();
    }
}