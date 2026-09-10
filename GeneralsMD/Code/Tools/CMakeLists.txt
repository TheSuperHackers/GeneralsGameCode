# Build tools

# Build useful tool binaries.
if(RTS_BUILD_ZEROHOUR_TOOLS)
    add_subdirectory(ImagePacker)
    add_subdirectory(MapCacheBuilder)
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    add_subdirectory(Autorun)
    add_subdirectory(Launcher)
    add_subdirectory(PATCHGET)
endif()
