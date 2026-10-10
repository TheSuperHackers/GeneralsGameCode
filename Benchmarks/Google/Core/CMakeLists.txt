# Core GameEngine code needs a game to compile, so g_googlebenchmark and z_googlebenchmark compile these sources.
set(COREI_GOOGLEBENCHMARK_SRC
    GoogleBenchmarkMain.cpp
    GameEngine/Common/AsciiStringBenchmark.cpp
    GameEngine/Common/Diagnostic/SimulationMathBenchmark.cpp
    Libraries/Compression/CompressionManagerBenchmark.cpp
)

add_library(corei_googlebenchmark INTERFACE)

target_sources(corei_googlebenchmark INTERFACE ${COREI_GOOGLEBENCHMARK_SRC})

target_link_libraries(corei_googlebenchmark INTERFACE
    benchmark::benchmark
    comctl32
    depsi_googlebenchmark
    imm32
    vfw32
    winmm
)
