# FFmpeg is supplied by the vcpkg manifest.
find_package(FFMPEG REQUIRED)

set(FFMPEG_RUNTIME_DIRECTORY
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin"
    CACHE PATH "FFmpeg runtime DLL directory")
set(FFMPEG_DEBUG_RUNTIME_DIRECTORY
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin"
    CACHE PATH "FFmpeg debug runtime DLL directory")
