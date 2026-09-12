cmake_minimum_required(VERSION 4.4.3)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(engine_foundation LANGUAGES CXX)
    include(cmake/toolchain/validate.cmake)
endif()

# Asset-free entry point. Do not include the legacy game's platform/dependency
# configuration here: this build validates the modern engine in isolation.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

if(BUILD_TESTING)
    add_test(NAME gameplay_architecture COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/.."
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/architecture/gameplay.cmake")
    add_test(NAME gameplay_architecture_checker COMMAND "${CMAKE_COMMAND}"
        "-DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/architecture-checker-fixtures"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/architecture/gameplay.test.cmake")
    add_test(NAME engine_toolchain_validation COMMAND "${CMAKE_COMMAND}"
        "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
        "-DCMAKE_CXX_COMPILER_CLANG_SCAN_DEPS=${CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain/validate.test.cmake")
    add_test(NAME engine_toolchain_bootstrap COMMAND "${CMAKE_COMMAND}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain/bootstrap.test.cmake")
endif()

option(ENGINE_JOBS_BUILD_TESTS "Build headless job-system tests" ${BUILD_TESTING})
option(ENGINE_EVENTS_BUILD_TESTS "Build headless event-batch tests" ${BUILD_TESTING})
option(ENGINE_TIME_BUILD_TESTS "Build headless simulation-time tests" ${BUILD_TESTING})
option(ENGINE_ECS_BUILD_TESTS "Build headless ECS tests" ${BUILD_TESTING})
option(ENGINE_GAMEPLAY_BUILD_TESTS "Build headless gameplay tests" ${BUILD_TESTING})

add_subdirectory(jobs)
add_subdirectory(events)
add_subdirectory(time)
add_subdirectory(ecs)
add_subdirectory(gameplay)
