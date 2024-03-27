
add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE Aqua)

install(FILES $<TARGET_RUNTIME_DLLS:demo> DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS demo
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin
        LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/bin
        ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/lib)

set_target_properties(demo PROPERTIES
    WIN32_EXECUTABLE $<IF:$<CONFIG:Release>,ON,OFF>)