#pragma once

#include "Core/Core.h"
#include "EventSystem/Event.h"

struct GLFWwindow;

namespace Aqua
{
    class WindowImpl;

    class Window
    {
    public:
        AQUA_API Window(std::shared_ptr<EventQueue> queue);
        AQUA_API Window(uint32_t width, uint32_t height, std::string_view title, std::shared_ptr<EventQueue> queue);

        Window(const Window&) = delete;
        Window(Window&&) noexcept = default;

        ~Window();

        AQUA_API bool update() const;
        AQUA_API bool handle_events() const;

        uint32_t get_width() const;
        uint32_t get_height() const;
        GLFWwindow* get_internal_handle() const;

        AQUA_API static bool Startup();
        AQUA_API static bool Shutdown();
    private:
        std::unique_ptr<WindowImpl> impl_;
        std::shared_ptr<EventQueue> queue_;
    };
}