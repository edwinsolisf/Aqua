#include "Application/Application.h"
#include "Renderer/Renderer.h"
#include "Window/Window.h"
#include "Debug/Debug.h"
#include "EventSystem/Event.h"

namespace Aqua
{
    class ApplicationImpl
    {
    public:
        ApplicationImpl()
        {
            running_ = true;

            AQUA_INFO("Current root path: " + Application::get_root_path().string());
            AQUA_INFO("Current application path: " + Application::get_binary_path().string());

            event_queue_ = std::make_unique<EventQueue>();

            if (!Window::Startup()) AQUA_CRITICAL("Window library initialization failure");
            window_ = std::make_unique<Window>(event_queue_);

            if (!Renderer::Startup()) AQUA_CRITICAL("Renderer initialization failure");
            renderer_ = std::make_unique<Renderer>(*window_, event_queue_);

            if (!renderer_->is_valid())
                AQUA_CRITICAL("Renderer creation failure");
        }

        ~ApplicationImpl()
        {
            renderer_ = nullptr;
            window_ = nullptr;

            Window::Shutdown();
            Renderer::Shutdown();

            running_ = false;
        }

        void handle_events()
        {
            while(!event_queue_->is_empty() && running_)
            {
                bool handled = window_->handle_events();
                if (handled) continue;
                
                handled = renderer_->handle_events();
                if (handled) continue;

                event_queue_->handle([this](const Event& e){
                    if (e.get_type() == Event::Types::Window_Closed)
                        running_ = false;
                    return true;
                });
            }
        }

        std::unique_ptr<Window> window_;
        std::unique_ptr<Renderer> renderer_;
        std::shared_ptr<EventQueue> event_queue_;
    
        bool running_ = false;
    };

    Application::Application(int argc, char** argv)
    {
        AQUA_PROFILE_BEGIN("file.txt");

        AQUA_PROFILE_FUNCTION();
        
        runtime_path_ = argv[0];
        current_application_ = this;

        impl_ = std::make_unique<ApplicationImpl>();
    }

    Application::~Application()
    {
        AQUA_PROFILE_END();

        current_application_ = nullptr;
    }

    void Application::run()
    {
        while(impl_->running_)
        {
            impl_->renderer_->render();
            impl_->window_->update();
            impl_->handle_events();
        }
    }

    Window& Application::get_window() { return *(impl_->window_); }
    const Window& Application::get_window() const { return *(impl_->window_); }

    Renderer& Application::get_renderer() { return *(impl_->renderer_); }
    const Renderer& Application::get_renderer() const { return *(impl_->renderer_); }
}