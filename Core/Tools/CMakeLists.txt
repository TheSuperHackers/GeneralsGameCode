# Build tools

# Build useful tool binaries.
if(RTS_BUILD_CORE_TOOLS)
    # add_subdirectory(W3DView)
endif()

# Build less useful tool/test binaries.
if(RTS_BUILD_CORE_EXTRAS)
    add_subdirectory(Compress)
    add_subdirectory(CRCDiff)
    add_subdirectory(matchbot)
    add_subdirectory(WW3D)
endif()

# REMOVE THIS WHEN CORE IS PRISTINE
# Add library interfaces here
if(RTS_BUILD_GENERALS_TOOLS OR RTS_BUILD_ZEROHOUR_TOOLS)
add_subdirectory(W3DView)
endif()

# REMOVE THIS WHEN CORE IS PRISTINE
# Add library interfaces here
if(RTS_BUILD_GENERALS_EXTRAS OR RTS_BUILD_ZEROHOUR_EXTRAS)
endif()
