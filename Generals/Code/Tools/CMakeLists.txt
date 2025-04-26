# Build tools

# Build useful tool binaries.
if(RTS_BUILD_GENERALS_TOOLS)
    add_subdirectory(GUIEdit)
    add_subdirectory(ImagePacker)
    add_subdirectory(MapCacheBuilder)
    add_subdirectory(ParticleEditor)
    add_subdirectory(WorldBuilder)
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_GENERALS_EXTRAS)
    add_subdirectory(Autorun)
    add_subdirectory(Launcher)
    add_subdirectory(PATCHGET)
endif()
