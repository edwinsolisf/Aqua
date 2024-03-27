#pragma once

#include "Core/Core.h"
#include "Window/Window.h"

namespace Aqua
{
    namespace Vulkan
    {
        class Renderer;
    }

    class AQUA_API Renderer
    {
    public:
        Renderer(Window& window, std::shared_ptr<EventQueue> queue);
        ~Renderer();

        Renderer(const Renderer&) = delete;

        bool is_valid() const noexcept;
        void render() const;

        static bool Startup();
        static bool Shutdown();

        bool handle_events() const;

        std::unique_ptr<Vulkan::Renderer> handle_;
        std::shared_ptr<EventQueue> queue_;
    };
}