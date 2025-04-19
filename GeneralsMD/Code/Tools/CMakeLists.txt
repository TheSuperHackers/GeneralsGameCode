# Build tools

# Build useful tool binaries.
if(RTS_BUILD_ZEROHOUR_TOOLS)
    add_subdirectory(DebugWindow)
    add_subdirectory(GUIEdit)
    add_subdirectory(ImagePacker)
    add_subdirectory(MapCacheBuilder)
    add_subdirectory(ParticleEditor)
    add_subdirectory(W3DView)
    add_subdirectory(wdump)
    add_subdirectory(WorldBuilder)
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_ZEROHOUR_EXTRAS)
    add_subdirectory(assetcull)
    add_subdirectory(Autorun)
    add_subdirectory(Launcher)
    add_subdirectory(mangler)
    add_subdirectory(PATCHGET)
    add_subdirectory(wolSetup)
endif()
