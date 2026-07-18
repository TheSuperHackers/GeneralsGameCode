// Opt-in CTest + doctest unit test harness entry point.
// Built only when -DRTS_BUILD_TESTS=ON is passed to CMake.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
