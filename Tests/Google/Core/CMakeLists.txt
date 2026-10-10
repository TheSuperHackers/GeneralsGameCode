# Core GameEngine code needs a game to compile, so g_googletest and z_googletest compile these sources.
set(COREI_GOOGLETEST_SRC
    GoogleTestMain.cpp
    GameEngine/Common/AsciiStringTest.cpp
    GameEngine/Common/INITest.cpp
    Libraries/Compression/CompressionManagerTest.cpp
)

add_library(corei_googletest INTERFACE)

target_sources(corei_googletest INTERFACE ${COREI_GOOGLETEST_SRC})

target_link_libraries(corei_googletest INTERFACE
    comctl32
    depsi_googletest
    GTest::gtest
    imm32
    vfw32
    winmm
)
