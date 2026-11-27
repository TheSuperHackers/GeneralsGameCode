# Contains options for VCPKG features

option(RTS_BUILD_OPTION_FFMPEG "Enable FFmpeg support" OFF)
if(RTS_BUILD_OPTION_FFMPEG)
    list(APPEND VCPKG_MANIFEST_FEATURES "ffmpeg")
endif()

option(RTS_BUILD_OPTION_PROFILE_TRACY "Build code with Tracy profiling enabled." OFF)
if(RTS_BUILD_OPTION_PROFILE_TRACY)
    list(APPEND VCPKG_MANIFEST_FEATURES "tracy")
endif()
