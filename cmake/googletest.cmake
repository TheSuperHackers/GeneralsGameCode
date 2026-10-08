# Fetch Google Test for the unit test executables.
# It is always built from source, so that it uses the same compiler, architecture and runtime library as the game.

set(BUILD_GMOCK OFF)

FetchContent_Declare(
    googletest
    EXCLUDE_FROM_ALL # Exclude Google Test's development files from the INSTALL target.
    GIT_REPOSITORY https://github.com/TheSuperHackers/google-test
    GIT_TAG        063de7e9578f82b369302001269680b4b1553359 # 1.18.0
)

FetchContent_MakeAvailable(googletest)
