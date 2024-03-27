add_library(stb STATIC)
target_sources(stb PRIVATE
    stb/stb_image.cpp)
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})