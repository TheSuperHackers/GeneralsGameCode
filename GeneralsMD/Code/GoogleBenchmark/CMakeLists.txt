# Keep the benchmark executable out of the GeneralsMD output folder, which CI collects into the build artifacts.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(z_googlebenchmark)

target_link_libraries(z_googlebenchmark PRIVATE
    corei_googlebenchmark
    z_gameengine
    z_gameenginedevice
    zi_always
)
