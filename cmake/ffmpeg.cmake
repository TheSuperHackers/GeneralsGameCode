include(FetchContent)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FFMPEG_ARCH x64)
else()
    set(FFMPEG_ARCH x86)
endif()

FetchContent_Declare(
    ffmpeg
    URL https://github.com/System233/ffmpeg-msvc-prebuilt/releases/download/ffmpeg-9.0.1/ffmpeg-9.0.1_${FFMPEG_ARCH}-windows-shared-lgpl.zip
    DOWNLOAD_NAME ffmpeg-${FFMPEG_ARCH}-windows-shared-lgpl.zip
)

FetchContent_MakeAvailable(ffmpeg)

set(FFMPEG_INCLUDE_DIRS "${ffmpeg_SOURCE_DIR}/include")
set(FFMPEG_LIBRARY_DIRS "${ffmpeg_SOURCE_DIR}/lib")
set(FFMPEG_LIBRARIES
    avcodec
    avformat
    avutil
    swscale
    swresample
)
