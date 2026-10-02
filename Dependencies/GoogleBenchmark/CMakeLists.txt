# g_googlebenchmark and z_googlebenchmark compile these sources together with the Core benchmarks.
set(DEPSI_GOOGLEBENCHMARK_SRC
    Utility/stringex_benchmark.cpp
)

add_library(depsi_googlebenchmark INTERFACE)

target_sources(depsi_googlebenchmark INTERFACE ${DEPSI_GOOGLEBENCHMARK_SRC})

target_link_libraries(depsi_googlebenchmark INTERFACE
    deps_utility
)
