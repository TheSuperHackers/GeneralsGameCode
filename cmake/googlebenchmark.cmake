# Fetch Google Benchmark for the benchmark executables.
# It is always built from source, so that it uses the same compiler, architecture and runtime library as the game.

# Its own tests would need Google Test and would register with our CTest.
set(BENCHMARK_ENABLE_TESTING OFF)
# Its MSVC warning level comes with -WX, and its upstream does not build with 32-bit MSVC.
set(BENCHMARK_ENABLE_WERROR OFF)

FetchContent_Declare(
    googlebenchmark
    EXCLUDE_FROM_ALL # Exclude Google Benchmark's development files from the INSTALL target.
    GIT_REPOSITORY https://github.com/TheSuperHackers/google-benchmark
    GIT_TAG        192ef10025eb2c4cdd392bc502f0c852196baa48 # 1.9.5
)

FetchContent_MakeAvailable(googlebenchmark)
