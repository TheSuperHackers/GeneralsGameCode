# Opt-in unit test harness for the openSAGE (GeneralsGameCode) engine.
# Enabled via -DRTS_BUILD_TESTS=ON. Not wired into any preset or CI by default.

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.12
)
FetchContent_MakeAvailable(doctest)

add_executable(core_tests
    test_main.cpp
    test_compression.cpp
)

target_link_libraries(core_tests PRIVATE
    doctest::doctest
    core_compression
    corei_libraries_include
    core_utility
)

target_compile_features(core_tests PRIVATE cxx_std_17)

add_test(NAME core_tests COMMAND core_tests)
