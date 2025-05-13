add_library(benchmark INTERFACE)
target_include_directories(benchmark INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
