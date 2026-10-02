# Keep the test executable out of the GeneralsMD output folder, which CI collects into the build artifacts.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(z_googletest)

target_link_libraries(z_googletest PRIVATE
    corei_googletest
    z_gameengine
    z_gameenginedevice
    zi_always
)

add_test(NAME z_googletest COMMAND z_googletest)
