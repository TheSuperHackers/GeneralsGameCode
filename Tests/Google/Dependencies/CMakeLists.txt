# g_googletest and z_googletest compile these sources together with the Core tests.
set(DEPSI_GOOGLETEST_SRC
    ICU/utf8_test.cpp
    Utility/atomic_x86interlocked_test.cpp
    Utility/interlocked_x86asm_common.h
    Utility/interlocked_x86asm_test.cpp
    Utility/stringex_memcchr_common.h
    Utility/stringex_memcchr_portable_test.cpp
    Utility/stringex_memcchr_sse2_test.cpp
    Utility/stringex_memcchr_x86asm_test.cpp
    Utility/stringex_test.cpp
)

add_library(depsi_googletest INTERFACE)

target_sources(depsi_googletest INTERFACE ${DEPSI_GOOGLETEST_SRC})

target_link_libraries(depsi_googletest INTERFACE
    deps_icu
    deps_utility
)

# Runs the utf8 tests against linked ICU, while g_googletest and z_googletest run them against IcuLoader.
if(TARGET deps_icu_linked)
    add_executable(icu_linked_googletest ICU/utf8_test.cpp)
    target_link_libraries(icu_linked_googletest PRIVATE
        depsi_always
        deps_icu_linked
        GTest::gtest_main
    )
    add_test(NAME icu_linked_googletest COMMAND icu_linked_googletest)
endif()
