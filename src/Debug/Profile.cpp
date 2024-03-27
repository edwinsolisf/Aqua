#include "Debug/Profile.h"

#include <chrono>

namespace Aqua
{
    Timer::Timer(const char* title)
        : title_(title), start_(std::chrono::high_resolution_clock::now())
    {
    }

    Timer::~Timer()
    {
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start_).count();

        Profiler::Get().info(std::string("Timer: ") + title_ + " took " + std::to_string(duration) + "us");
    }
}