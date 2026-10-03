# Keep the benchmark executable out of the Generals output folder, which CI collects into the build artifacts.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(g_googlebenchmark)

target_link_libraries(g_googlebenchmark PRIVATE
    corei_googlebenchmark
    g_gameengine
    g_gameenginedevice
    gi_always
)
