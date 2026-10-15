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
cmake --preset windows-x86-msvc19 -DRTS_BUILD_OPTION_TESTS=ON
cmake --build --preset windows-x86-msvc19
ctest --preset windows-x86-msvc19
```
Visual Studio also lists the tests in its Test Explorer. A test executable can be run directly as well, for example with `--gtest_filter=AsciiString.*` or with `--gtest_break_on_failure` to stop in the debugger.

Each game has one test executable, g_googletest for Generals and z_googletest for Zero Hour. It compiles the tests of all layers:

| Sources | Tests |
|---|---|
| Tests/Google/Dependencies | Dependencies code. It must not use Core. |
| Tests/Google/Core | Core game engine and libraries. |

Use C++ define RTS_GENERALS when adding game specific tests to Core.

A test file is named after the file it tests and follows its naming style, for example stringex_test.cpp for stringex.h and AsciiStringTest.cpp for AsciiString.h.

A failed `DEBUG_ASSERTCRASH` fails the running test in builds with debug crashing, which are the debug presets and builds with `RTS_DEBUG_CRASHING=ON`. Release builds compile these asserts out, so a test must not rely on them alone. CI runs the tests in release, profile and debug builds.

A `RELEASE_CRASH` reports its reason as a failure of the running test and then exits the test executable. Expected release crashes can be tested with `EXPECT_EXIT(..., ::testing::ExitedWithCode(1), "")`.

The test executables use the regular user data folder of the game, so a `RELEASE_CRASH` also writes its ReleaseCrashInfo.txt there.

# Benchmarks

Benchmarks use [Google Benchmark](https://github.com/TheSuperHackers/google-benchmark). Enable them with `RTS_BUILD_OPTION_BENCHMARKS`. They are organized like the unit tests under Benchmarks/Google, with the executables g_googlebenchmark and z_googlebenchmark. CI builds them, but does not run them.

Run benchmarks from a Release build. Select benchmarks with `--benchmark_filter=<regex>`, and use `--benchmark_out=<file>.json` to compare the results before and after a change.

The `BM_INIScan` benchmarks compare the production numeric scanners with the original `sscanf` conversion and failure check. Ordinary inputs exercise `from_chars`; overflow and underflow inputs exercise the range fallback. Each iteration parses four fixed tokens, so divide the reported CPU time by four for ns/token. `items_per_second` already counts individual tokens. Range-error `sscanf` results characterize the local CRT and are not portable C behavior. These measurements do not represent game startup time or the frequency of range errors in game data.

For example, from an x86 developer shell:
```
cmake --preset windows-x86-msvc19 -DRTS_BUILD_OPTION_BENCHMARKS=ON
cmake --build --preset windows-x86-msvc19 --target z_googlebenchmark g_googlebenchmark
build/windows-x86-msvc19/Benchmarks/Google/Release/z_googlebenchmark.exe --benchmark_filter=^BM_INIScan --benchmark_min_time=0.2s --benchmark_repetitions=5 --benchmark_enable_random_interleaving=true --benchmark_out=ini-benchmark.json --benchmark_out_format=json
```
Use the median CPU time for each case when comparing repeated runs. The same filter also works with `g_googlebenchmark.exe`. Relative output paths are resolved from the executable's directory; use an absolute `--benchmark_out` path to save elsewhere.
