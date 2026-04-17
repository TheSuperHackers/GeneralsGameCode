set(GM_ENABLE_INTRINSICS OFF CACHE BOOL "Disable intrinsics for cross-arch determinism" FORCE)
set(GM_ENABLE_TESTS OFF CACHE BOOL "Disable GameMath tests" FORCE)

FetchContent_Declare(
    gamemath
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GameMath.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(gamemath)

include_directories(${gamemath_SOURCE_DIR}/include)
