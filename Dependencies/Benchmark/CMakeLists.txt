cmake_minimum_required(VERSION 3.5.0)

project(benchmark VERSION 1.0.0 LANGUAGES C)

add_library(benchmark INTERFACE)
target_include_directories(benchmark INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
