#include "Utils/ShaderCompilation.h"

#include "Debug/Debug.h"

// #include <glslang/Include/ResourceLimits.h>
// #include <glslang/Public/ShaderLang.h>
// #include <glslang/SPIRV/GlslangToSpv.h>
#include <shaderc/shaderc.h>

#include <fstream>

namespace Aqua
{

    /*
       Deduce the language from the filename.  Files must end in one of the
       following extensions:
    
       .vert = vertex
       .tesc = tessellation control
       .tese = tessellation evaluation
       .geom = geometry
       .frag = fragment
       .comp = compute
       .rgen = ray generation
       .rint = ray intersection
       .rahit = ray any hit
       .rchit = ray closest hit
       .rmiss = ray miss
       .rcall = ray callable
       .mesh  = mesh
       .task  = task

       Additionally, the file names may end in .<stage>.glsl and .<stage>.hlsl
       where <stage> is one of the stages listed above.
    */
    static shaderc_shader_kind find_shader_stage(std::string_view name)
    {
        std::string stageName;
            
        size_t firstExtStart = name.find_last_of(".");
        bool hasFirstExt = firstExtStart != std::string::npos;

        size_t secondExtStart = hasFirstExt ? name.find_last_of(".", firstExtStart - 1) : std::string::npos;
        bool hasSecondExt = secondExtStart != std::string::npos;

        auto firstExt = std::string(name.substr(firstExtStart + 1, std::string::npos));
        bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");
        
        // if (usesUnifiedExt && firstExt == "hlsl")
        //     Options |= EOptionReadHlsl;

        if (hasFirstExt && !usesUnifiedExt)
        {
            stageName = firstExt;
        }
        else if (usesUnifiedExt && hasSecondExt)
        {
            stageName = name.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);
        }
        else 
        {
            AQUA_ERROR("Shader Compilation Error: shader filename is malformed");
            return shaderc_shader_kind::shaderc_vertex_shader;
        }

        if (stageName == "vert")
            return shaderc_shader_kind::shaderc_vertex_shader;
        else if (stageName == "tesc")
            return shaderc_shader_kind::shaderc_tess_control_shader;
        else if (stageName == "tese")
            return shaderc_shader_kind::shaderc_tess_evaluation_shader;
        else if (stageName == "geom")
            return shaderc_shader_kind::shaderc_geometry_shader;
        else if (stageName == "frag")
            return shaderc_shader_kind::shaderc_fragment_shader;
        else if (stageName == "comp")
            return shaderc_shader_kind::shaderc_compute_shader;
        else if (stageName == "rgen")
            return shaderc_shader_kind::shaderc_raygen_shader;
        else if (stageName == "rint")
            return shaderc_shader_kind::shaderc_intersection_shader;
        else if (stageName == "rahit")
            return shaderc_shader_kind::shaderc_anyhit_shader;
        else if (stageName == "rchit")
            return shaderc_shader_kind::shaderc_closesthit_shader;
        else if (stageName == "rmiss")
            return shaderc_shader_kind::shaderc_miss_shader;
        else if (stageName == "rcall")
            return shaderc_shader_kind::shaderc_callable_shader;
        else if (stageName == "mesh")
            return shaderc_shader_kind::shaderc_mesh_shader;
        else if (stageName == "task")
            return shaderc_shader_kind::shaderc_task_shader;

        AQUA_ERROR("[Shader Compilation Error]: file extension " + stageName + " is not among the available options");

        return shaderc_shader_kind::shaderc_vertex_shader;
    }

    std::vector<uint32_t> compile_shader_from_file(const std::filesystem::path& file_path)
    {
        std::ifstream file(file_path.string());

        if (!file.is_open())
        {
            AQUA_ERROR("[Shader Compilation Error]: cannot open file " + file_path.string());
            return {};
        }

        std::stringstream stream;
        stream << file.rdbuf();

        auto source = stream.str();

        auto filename = file_path.filename().string();
        auto compiler = shaderc_compiler_initialize();
        auto stage = find_shader_stage(filename);
        auto compiler_options = shaderc_compile_options_initialize();

        auto result = shaderc_compile_into_spv(compiler,
                                               source.c_str(),
                                               source.size(),
                                               stage,
                                               filename.c_str(),
                                               "",
                                               compiler_options);

        shaderc_compile_options_release(compiler_options);
        shaderc_compiler_release(compiler);

        if (shaderc_result_get_compilation_status(result) != 
            shaderc_compilation_status::shaderc_compilation_status_success)
        {
            std::string error_message = shaderc_result_get_error_message(result);
            AQUA_ERROR("[Shader Compilation Error]: " + error_message);
            return {};
        }

        auto byte_size = shaderc_result_get_length(result);
        auto byte_code = shaderc_result_get_bytes(result);

        std::vector<uint32_t> data(reinterpret_cast<const uint32_t*>(byte_code),
                                   reinterpret_cast<const uint32_t*>(byte_code + byte_size));

        AQUA_INFO("[Shader Compilation Info]: " + filename + " shader was compiled successfully");

        return data;
    }
}