
set(AQUA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(AQUA_INCLUDE_HEADERS
        Application/Application.h
        Core/Core.h
        Core/Platform.h
        Debug/Debug.h
        Debug/Profile.h
        EventSystem/Event.h
        Renderer/Renderer.h
        Renderer/Vulkan/VulkanCore.h
        Renderer/Vulkan/VulkanBuffer.h
        Renderer/Vulkan/VulkanBufferBase.h
        Renderer/Vulkan/VulkanDebug.h
        Renderer/Vulkan/VulkanRenderer.h
        Renderer/Vulkan/VulkanTexture.h
        Utils/ShaderCompilation.h
        Window/Window.h
        Window/WindowInternal.h
        )

add_subdirectory(Math)

list(TRANSFORM AQUA_INCLUDE_HEADERS PREPEND ${AQUA_INCLUDE_DIR}/)

set(AQUA_INCLUDE_DIR ${AQUA_INCLUDE_DIR} PARENT_SCOPE)
set(AQUA_INCLUDE_HEADERS ${AQUA_INCLUDE_HEADERS} PARENT_SCOPE)