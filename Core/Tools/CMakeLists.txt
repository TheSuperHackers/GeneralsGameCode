# Build tools

# Build useful tool binaries.
if(RTS_BUILD_CORE_TOOLS)
    # TODO: add subdirectory...
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_CORE_EXTRAS)
    add_subdirectory(buildVersionUpdate)
    add_subdirectory(Compress)
    add_subdirectory(CRCDiff)
    add_subdirectory(matchbot)
    add_subdirectory(textureCompress)
    add_subdirectory(timingTest)
    add_subdirectory(versionUpdate)
    add_subdirectory(WW3D)
endif()
