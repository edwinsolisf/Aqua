#pragma once

#include "Core/Core.h"

namespace Aqua
{
    std::vector<uint32_t> compile_shader_from_file(const std::filesystem::path& file_path);
}