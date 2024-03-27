cmake_minimum_required(VERSION 3.20)

project(
	Aqua
	VERSION 1.0
	LANGUAGES CXX
)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS glslang SPIRV-Tools shaderc_combined)

option(BREAK_ON_ERROR OFF)
option(BUILD_SHARED_LIBS "Build dynamics library" OFF)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(vendor)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(demos)

# Link assets folder inside the build folder
if (NOT EXISTS ${CMAKE_BINARY_DIR}/bin/assets)
	set(LINK_RESULT 1)
	file(CREATE_LINK ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/bin/assets RESULT LINK_RESULT SYMBOLIC)

	if (${LINK_RESULT} EQUAL 0)
		message("Created link ${CMAKE_BINARY_DIR}/bin/assets")
	else()
		message(FATAL_ERROR "Could not create link to assets, retry running CMAKE with Admin Privileges")
	endif()
endif()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/assets)
	set(LINK_RESULT 1)
	file(CREATE_LINK ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets RESULT LINK_RESULT SYMBOLIC)

	if (${LINK_RESULT} EQUAL 0)
		message("Created link ${CMAKE_BINARY_DIR}/bin/assets")
	else()
		message(FATAL_ERROR "Could not create link to assets, retry running CMAKE with Admin Privileges")
	endif()
endif()

