# Keep the test executable out of the Generals output folder, which CI collects into the build artifacts.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(g_googletest)

target_link_libraries(g_googletest PRIVATE
    corei_googletest
    g_gameengine
    g_gameenginedevice
    gi_always
)

add_test(NAME g_googletest COMMAND g_googletest)
