#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "EventSystem/Event.h"

namespace Aqua
{
    class WindowImpl
    {
    public:
        WindowImpl(std::shared_ptr<EventQueue> queue)
        {
            auto monitor = glfwGetPrimaryMonitor();

            int width = 0;
            int height = 0;
            glfwGetMonitorWorkarea(monitor, nullptr, nullptr, &width, &height);

            SetWindowHints();
            window_ = glfwCreateWindow(width / 2, height / 2, "Window", nullptr, nullptr);
            glfwSetWindowUserPointer(window_, static_cast<void*>(queue.get()));

            SetWindowCallbacks();
        }
        
        WindowImpl(uint32_t width, uint32_t height, std::string_view title, std::shared_ptr<EventQueue> queue)
            : title_{ title }
        {
            SetWindowHints();
            window_ = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
            glfwSetWindowUserPointer(window_, static_cast<void*>(queue.get()));

            SetWindowCallbacks();
        }

        void SetWindowCallbacks() const
        {
            glfwSetKeyCallback(window_,
                [](GLFWwindow* window, int key, int scancode, int action, int mods){
                    EventQueue& queue = *static_cast<EventQueue*>(glfwGetWindowUserPointer(window));
                
                    KeyData data = { .key = KeyCode(key) };
                    switch (static_cast<KeyAction>(action))
                    {
                    case KeyAction::Release:
                        queue.enqueue(Event::KeyReleaseEvent(data));
                    case KeyAction::Press:
                        queue.enqueue(Event::KeyPressEvent(data));
                    case KeyAction::Repeat:
                        queue.enqueue(Event::KeyRepeatEventData(data));
                    }
                });

            glfwSetMouseButtonCallback(window_,
                [](GLFWwindow* window, int button, int action, int mods){
                    EventQueue& queue = *static_cast<EventQueue*>(glfwGetWindowUserPointer(window));

                    queue.enqueue(Event::MouseClickEvent({ MouseCode(button), MouseAction(action) }));
                });

            glfwSetCursorPosCallback(window_,
                [](GLFWwindow* window, double xpos, double ypos){
                    EventQueue& queue = *static_cast<EventQueue*>(glfwGetWindowUserPointer(window));

                    queue.enqueue(Event::MouseMoveEvent({xpos, ypos}));
                });

            glfwSetWindowSizeCallback(window_,
                [](GLFWwindow* window, int width, int height){
                    EventQueue& queue = *static_cast<EventQueue*>(glfwGetWindowUserPointer(window));

                    queue.enqueue(Event::WindowResizeEvent({width, height}));
                });

            glfwSetWindowCloseCallback(window_,
                [](GLFWwindow* window){
                    EventQueue& queue = *static_cast<EventQueue*>(glfwGetWindowUserPointer(window));

                    queue.enqueue(Event::WindowCloseEvent());
                });
        }

        ~WindowImpl()
        {
            glfwDestroyWindow(window_);
        }

        void SetWindowHints() const
        {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        uint32_t get_width() const
        {
            int width = 0;
            glfwGetWindowSize(window_, &width, nullptr);

            return static_cast<uint32_t>(width);
        }

        uint32_t get_height() const
        {
            int height = 0;
            glfwGetWindowSize(window_, nullptr, &height);

            return static_cast<uint32_t>(height);
        }

        void update() const
        {
            glfwPollEvents();
            // glfwWaitEvents();
        }

        bool should_close() const
        {
            return static_cast<bool>(glfwWindowShouldClose(window_));
        }

        static bool Startup()
        {
            return glfwInit();
        }

        static bool Shutdown()
        {
            glfwTerminate();

            return true;
        }

        GLFWwindow* get_glfw_window() const noexcept { return window_; }

    private:
        GLFWwindow* window_;
        std::string title_;
    };
}