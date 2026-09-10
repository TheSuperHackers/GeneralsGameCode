# Build tools

# Build useful tool binaries.
if(RTS_BUILD_CORE_TOOLS)
    add_subdirectory(DebugWindow)
    add_subdirectory(ParticleEditor)
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_CORE_EXTRAS)
    add_subdirectory(assetcull)
    add_subdirectory(Babylon)
    add_subdirectory(buildVersionUpdate)
    add_subdirectory(Compress)
    add_subdirectory(CRCDiff)
    add_subdirectory(mangler)
    add_subdirectory(matchbot)
    add_subdirectory(textureCompress)
    add_subdirectory(timingTest)
    add_subdirectory(versionUpdate)
    add_subdirectory(wolSetup)
    add_subdirectory(WW3D)
endif()

# Add the GeneralsMD tools that are supported by the active build.
if(RTS_BUILD_ZEROHOUR_TOOLS)
    add_subdirectory(ImagePacker)
    add_subdirectory(MapCacheBuilder)
endif()

# Add GeneralsMD extra tools when requested.
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    add_subdirectory(Autorun)
    add_subdirectory(Launcher)
    add_subdirectory(PATCHGET)
endif()
