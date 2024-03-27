#pragma once

#include "Core/Core.h"

namespace Aqua
{
    class ApplicationImpl;
    class Renderer;
    class Window;

    class Application
    {
    public:
        AQUA_API Application(int argc, char** argv);
        AQUA_API ~Application();

        AQUA_API virtual void run();

        AQUA_API Window& get_window();
        AQUA_API const Window& get_window() const;

        AQUA_API Renderer& get_renderer();
        AQUA_API const Renderer& get_renderer() const;

        static std::filesystem::path get_binary_path() { return std::filesystem::absolute("."); }
        static std::filesystem::path get_root_path() { return std::filesystem::absolute(".."); }
        static std::filesystem::path get_assets_path() { return get_root_path() / "assets"; }
        static std::filesystem::path get_runtime_path() { return runtime_path_; }

        AQUA_API static Application& get() { return *current_application_; }

    private:
        std::unique_ptr<ApplicationImpl> impl_;

        inline static std::filesystem::path runtime_path_;
        inline static Application* current_application_ = nullptr;
    };
}