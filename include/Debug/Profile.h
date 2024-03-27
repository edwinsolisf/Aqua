#pragma once

#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include "Core/Platform.h"

#ifdef AQUA_PLATFORM_WINDOWS
    #include <windows.h>
#endif

namespace Aqua
{
    class Profiler
    {
    public:
        Profiler(std::string_view filename)
            : file_stream_{ filename.data() }
        {
            if (file_stream_.fail())
                error("Could not create profiling file");
        }

        Profiler(const Profiler&) = delete;
        Profiler(Profiler&&) = default;
        
        ~Profiler() = default;


        template<typename T>
        void info(T&& in)
        {
            std::lock_guard lock{ mutex_ };

            std::cout << "[  INFO  ]: " << in << '\n';
            file_stream_<< "[  INFO  ]: " << in << '\n';
        }

        template<typename T>
        void warn(T&& in)
        {
            std::lock_guard lock{ mutex_ };

            std::cout << color_yellow << "[WARNING ]: " << in << color_reset << '\n';
            file_stream_ << "[WARNING ]: " << std::forward<T>(in) << '\n';
        }

        template<typename T>
        void error(T&& in)
        {
            std::lock_guard lock{ mutex_ };

            std::cout << color_red << "[  ERROR ]: " << in << color_reset << '\n';
            file_stream_ << "[  ERROR ]: " << std::forward<T>(in) << '\n';
        }

        template<typename T>
        void critical(T&& in)
        {
            std::lock_guard lock{ mutex_ };

            std::cout << color_bright_red << "[CRITICAL]: " << in << color_reset << '\n';
            file_stream_ << "[CRITICAL]: " << std::forward<T>(in) << '\n';
        }

        static void BeginProfile(std::string_view filename)
        {
            std::lock_guard lock{ mutex_ };

            #ifdef AQUA_PLATFORM_WINDOWS
                DWORD dwMode;
                HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                GetConsoleMode(hOutput, &dwMode);
                dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOutput, dwMode);
            #endif

            if (current_profiler)
            {
                current_profiler->error("Profiler already open");
                return;
            }

            current_profiler = std::make_unique<Profiler>(filename);
        }

        static void EndProfile()
        {
            std::lock_guard lock{ mutex_ };
            current_profiler = nullptr;
        }

        static Profiler& Get() { return *current_profiler; }

    private:
        std::ofstream file_stream_;

        inline static std::mutex mutex_;
        inline static std::unique_ptr<Profiler> current_profiler = nullptr;

        const char* color_reset = "\033[0m";
        const char* color_bright_red = "\x1B[91m";
        const char* color_red = "\x1B[31m";
        const char* color_yellow = "\x1B[93m";
        const char* color_white = "\x1B[97m";
    };

    class Timer
    {
    public:
        Timer(const char* title);

        Timer(const Timer&) = delete;
        Timer(Timer&&) = default;

        ~Timer();
    private:
        const char* title_;
        std::chrono::high_resolution_clock::time_point start_;
    };
}