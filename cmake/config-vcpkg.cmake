# Contains options for VCPKG features

option(RTS_BUILD_OPTION_FFMPEG "Enable FFmpeg support" OFF)
if(RTS_BUILD_OPTION_FFMPEG)
    list(APPEND VCPKG_MANIFEST_FEATURES "ffmpeg")
endif()

# Windows games load the OS icu.dll at runtime. Only the unit tests link ICU from vcpkg.
if(RTS_BUILD_OPTION_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()
