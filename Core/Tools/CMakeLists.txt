# Build tools

# Build useful tool binaries.
if(RTS_BUILD_CORE_TOOLS)
    # TODO: add subdirectory...
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_CORE_EXTRAS)
    add_subdirectory(Compress)
    add_subdirectory(CRCDiff)
    add_subdirectory(WW3D)
endif()
