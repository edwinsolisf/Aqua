
add_library(Aqua)

# sources
target_sources(Aqua PRIVATE
                Application/Application.cpp
                Debug/Profile.cpp
                Renderer/Renderer.cpp
                Renderer/Vulkan/VulkanDebug.cpp
                Renderer/Vulkan/VulkanBuffer.cpp
                Renderer/Vulkan/VulkanBufferBase.cpp
                Renderer/Vulkan/VulkanDevice.cpp
                Renderer/Vulkan/VulkanImage.cpp
                Renderer/Vulkan/VulkanRenderer.cpp
                Renderer/Vulkan/VulkanTexture.cpp
                Utils/ShaderCompilation.cpp
                Window/Window.cpp)

# headers
target_sources(Aqua PUBLIC ${AQUA_INCLUDE_HEADERS})

message("${AQUA_INCLUDE_DIR}")
foreach(source ${AQUA_INCLUDE_HEADERS})
    message(${source})
endforeach()


target_link_libraries(Aqua PRIVATE glfw Vulkan::Vulkan Vulkan::shaderc_combined stm stb)
target_include_directories(Aqua PUBLIC ${AQUA_INCLUDE_DIR} ${Vulkan_INCLUDE_DIR})
target_compile_features(Aqua PUBLIC cxx_std_20)

if (${BUILD_SHARED_LIBS})
    target_compile_definitions(Aqua PRIVATE AQUA_BUILD_SHARED_API)
    target_compile_definitions(Aqua PUBLIC AQUA_SHARED_API)
endif()

target_compile_definitions(Aqua PRIVATE $<IF:$<OR:$<CONFIG:DEBUG>,$<CONFIG:RELWITHDEBINFO>>,AQUA_DEBUG,>)
if (${BREAK_ON_ERROR})
    target_compile_definitions(Aqua PRIVATE AQUA_ERROR_BREAK)
endif()

option(ENABLE_PROFILING "Enable Program Profiling" ON)
option(ENABLE_ASSERTS "Enable Program Assertions" ON)

if (${ENABLE_PROFILING})
    target_compile_definitions(Aqua PRIVATE AQUA_ENABLE_PROFILING)
endif()

if (${ENABLE_ASSERTS})
    target_compile_definitions(Aqua PRIVATE AQUA_ENABLE_ASSERTS)
endif()

install(TARGETS Aqua
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/bin
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)