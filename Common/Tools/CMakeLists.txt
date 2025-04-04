# Build tools

# Build useful tool binaries.
if(GENZH_BUILD_COMMON_TOOLS)
endif()

# Build less useful tool/test binaries.
if(GENZH_BUILD_COMMON_EXTRAS)
    add_subdirectory(assetcull)
    add_subdirectory(Compress)
    add_subdirectory(CRCDiff)
endif()
