# g_googletest and z_googletest compile these sources together with the Core tests.
set(DEPSI_GOOGLETEST_SRC
    Utility/stringex_memcchr_common.h
    Utility/stringex_memcchr_portable_test.cpp
    Utility/stringex_memcchr_sse2_test.cpp
    Utility/stringex_memcchr_x86asm_test.cpp
    Utility/stringex_test.cpp
)

add_library(depsi_googletest INTERFACE)

target_sources(depsi_googletest INTERFACE ${DEPSI_GOOGLETEST_SRC})

target_link_libraries(depsi_googletest INTERFACE
    deps_utility
)
