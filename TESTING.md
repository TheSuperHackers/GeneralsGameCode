# Test Replays

The GeneralsReplays folder contains replays and the required maps that are tested in CI to ensure that the game is retail compatible.

You can also test with these replays locally:
- Copy the replays into a subfolder in your `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays` folder.
- Copy the maps into `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Maps`
- Start the test with this: (copy into a .bat file next to your executable)
```
START /B /W generalszh.exe -jobs 4 -headless -replay subfolder/*.rep > replay_check.log
echo %errorlevel%
PAUSE
```
It will run the game in the background and check that each replay is compatible. You need to use a VC6 build with optimizations and RTS_BUILD_OPTION_DEBUG = OFF, otherwise the game won't be compatible.

# Unit Tests

Unit tests use [Google Test](https://github.com/TheSuperHackers/google-test). They need a C++17 compiler, so they are not available with VC6. Enable them with `RTS_BUILD_OPTION_TESTS` and run them with CTest:
```
cmake --preset win32 -DRTS_BUILD_OPTION_TESTS=ON
cmake --build --preset win32
ctest --preset win32
```
Visual Studio also lists the tests in its Test Explorer. A test executable can be run directly as well, for example with `--gtest_filter=AsciiString.*` or with `--gtest_break_on_failure` to stop in the debugger.

Each game has one test executable, g_googletest for Generals and z_googletest for Zero Hour. It compiles the tests of all layers:

| Sources | Tests |
|---|---|
| Tests/Google/Dependencies | Dependencies code. It must not use Core. |
| Tests/Google/Core | Core game engine and libraries. |

Use C++ define RTS_GENERALS when adding game specific tests to Core.

A test file is named after the file it tests and follows its naming style, for example stringex_test.cpp for stringex.h and AsciiStringTest.cpp for AsciiString.h.

A failed `DEBUG_ASSERTCRASH` fails the running test in builds with debug crashing, which are the debug presets and builds with `RTS_DEBUG_CRASHING=ON`. Release builds compile these asserts out, so a test must not rely on them alone.

A `RELEASE_CRASH` reports its reason as a failure of the running test and then exits the test executable. Expected release crashes can be tested with `EXPECT_EXIT(..., ::testing::ExitedWithCode(1), "")`.

The test executables use the regular user data folder of the game, so a `RELEASE_CRASH` also writes its ReleaseCrashInfo.txt there.
