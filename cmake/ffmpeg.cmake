include(FetchContent)

FetchContent_Declare(
    ffmpeg
    URL https://github.com/System233/ffmpeg-msvc-prebuilt/releases/download/ffmpeg-9.0.1/ffmpeg-9.0.1_x86-windows-shared-lgpl.zip
    DOWNLOAD_NAME ffmpeg-x86-windows-shared-lgpl.zip
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
