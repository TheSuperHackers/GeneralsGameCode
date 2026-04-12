FetchContent_Declare(
    fdlibm
    GIT_REPOSITORY https://github.com/Okladnoj/fdlibm-deterministic.git
    GIT_TAG        02d7d2c
)

FetchContent_MakeAvailable(fdlibm)

include_directories(${fdlibm_SOURCE_DIR})
