# Core GameEngine code needs a game to compile, so g_googletest and z_googletest compile these sources.
set(COREI_GOOGLETEST_SRC
    GoogleTestMain.cpp
    GameEngine/Common/AsciiStringTest.cpp
    GameEngine/Common/FileSystemTest.cpp
    GameEngine/Common/FileSystemTestFixture.h
    GameEngine/Common/INITest.cpp
    GameEngine/GameClient/DisplayStringDummyTest.cpp
    GameEngineDevice/StdDevice/Common/StdLocalFileSystemTest.cpp
    Libraries/Compression/CompressionManagerTest.cpp
    Libraries/WWVegas/WW3D2/hlod_test.cpp
    Libraries/WWVegas/WW3D2/soundrobj_test.cpp
    Libraries/WWVegas/WWMath/WWMathCalls.h
    Libraries/WWVegas/WWMath/WWMathDumps.h
    Libraries/WWVegas/WWMath/WWMathKeys.h
    Libraries/WWVegas/WWMath/WWMathTest.cpp
    Libraries/WWVegas/WWMath/WWMathTestCase.h
    Rendering/EXAMPLE_TEMPORARY_ClearTest.cpp
    Rendering/EXAMPLE_TEMPORARY_LaserTest.cpp
    Rendering/EXAMPLE_TEMPORARY_ParticlesTest.cpp
    Rendering/EXAMPLE_TEMPORARY_Render2DBlendingTest.cpp
    Rendering/EXAMPLE_TEMPORARY_Render2DShapesTest.cpp
    Rendering/EXAMPLE_TEMPORARY_Render2DTextureTest.cpp
    Rendering/EXAMPLE_TEMPORARY_TextTest.cpp
    Rendering/EXAMPLE_TEMPORARY_TracerTest.cpp
    Rendering/EXAMPLE_TEMPORARY_TriangleTest.cpp
    Rendering/RenderingTestFixture.cpp
    Rendering/RenderingTestFixture.h
)

add_library(corei_googletest INTERFACE)

target_sources(corei_googletest INTERFACE ${COREI_GOOGLETEST_SRC})

target_link_libraries(corei_googletest INTERFACE
    comctl32
    depsi_googletest
    GTest::gtest
    imm32
    stb
    vfw32
    winmm
)
