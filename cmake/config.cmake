### BUILD OPTIONS ###

# Do we want to build extra SDK stuff or just the game binary?
option(GENZH_BUILD_ZEROHOUR "Build Zero Hour code." ON)
option(GENZH_BUILD_GENERALS "Build Generals code." ON)
option(GENZH_BUILD_INTERNAL "Build code with the \"Internal\" configuration." OFF)
option(GENZH_BUILD_PROFILE "Build code with the \"Profile\" configuration." OFF)
option(GENZH_BUILD_DEBUG "Build code with the \"Debug\" configuration." OFF)

if(NOT GENZH_BUILD_ZEROHOUR AND NOT GENZH_BUILD_GENERALS)
    set(GENZH_BUILD_ZEROHOUR TRUE)
    message("You must select one project to build, building Zero Hour by default.")
endif()

add_feature_info(ZeroHourStuff GENZH_BUILD_ZEROHOUR "Build Zero Hour code")
add_feature_info(GeneralsStuff GENZH_BUILD_GENERALS "Build Generals code")
add_feature_info(InternalBuild GENZH_BUILD_INTERNAL "Building as an \"Internal\" build")
add_feature_info(ProfileBuild GENZH_BUILD_PROFILE "Building as a \"Profile\" build")
add_feature_info(DebugBuild GENZH_BUILD_DEBUG "Building as a \"Debug\" build")


### GAME MEMORY OPTIONS ###

# Game Memory features
option(GENZH_GAMEMEMORY_ENABLE "Enables the memory pool and dynamic memory allocator." ON)

# Memory pool features
option(GENZH_MEMORYPOOL_OVERRIDE_MALLOC "Enables the Dynamic Memory Allocator for malloc calls." OFF)
option(GENZH_MEMORYPOOL_MPSB_DLINK "Adds a backlink to MemoryPoolSingleBlock. Makes it faster to free raw DMA blocks, but increases memory consumption." ON)

# Memory pool debugs
option(GENZH_MEMORYPOOL_DEBUG "Enables Memory Pool debug." ON)
option(GENZH_MEMORYPOOL_DEBUG_CUSTOM_NEW "Enables a custom new operator for the Memory Pool." ON)
option(GENZH_MEMORYPOOL_DEBUG_CHECKPOINTING "Records checkpoint information about the history of memory allocations." OFF)
option(GENZH_MEMORYPOOL_DEBUG_BOUNDINGWALL "Enables bounding wall checks around memory chunks to find memory trampling." ON)
option(GENZH_MEMORYPOOL_DEBUG_STACKTRACE "Enables stack trace collection for allocations. Reduces runtime performance significantly." OFF)
option(GENZH_MEMORYPOOL_DEBUG_INTENSE_VERIFY "Enables intensive verifications after nearly every memory operation. OFF by default, since it slows down things a lot, but is worth turning on for really obscure memory corruption issues." OFF)
option(GENZH_MEMORYPOOL_DEBUG_CHECK_BLOCK_OWNERSHIP "Enables debug to verify that a block actually belongs to the pool it is called with. This is great for debugging, but can be realllly slow, so is OFF by default." OFF)
option(GENZH_MEMORYPOOL_DEBUG_INTENSE_DMA_BOOKKEEPING "Prints statistics for memory usage of Memory Pools." OFF)

# Game Memory features
add_feature_info(GameMemoryEnable GENZH_GAMEMEMORY_ENABLE "Build with the original game memory implementation")

# Memory pool features
add_feature_info(MemoryPoolOverrideMalloc GENZH_MEMORYPOOL_OVERRIDE_MALLOC "Build with Memory Pool malloc")
add_feature_info(MemoryPoolMpsbDlink GENZH_MEMORYPOOL_MPSB_DLINK "Build with Memory Pool backlink")

# Memory pool debugs
add_feature_info(MemoryPoolDebug GENZH_MEMORYPOOL_DEBUG "Build with Memory Pool debug")
add_feature_info(MemoryPoolDebugCustomNew GENZH_MEMORYPOOL_DEBUG_CUSTOM_NEW "Build with Memory Pool custom new")
add_feature_info(MemoryPoolDebugCheckpointing GENZH_MEMORYPOOL_DEBUG_CHECKPOINTING "Build with Memory Pool checkpointing")
add_feature_info(MemoryPoolDebugBoundingwall GENZH_MEMORYPOOL_DEBUG_BOUNDINGWALL "Build with Memory Pool Bounding Wall")
add_feature_info(MemoryPoolDebugStacktrace GENZH_MEMORYPOOL_DEBUG_STACKTRACE "Build with Memory Pool Stacktrace")
add_feature_info(MemoryPoolDebugIntenseVerify GENZH_MEMORYPOOL_DEBUG_INTENSE_VERIFY "Build with Memory Pool intense verify")
add_feature_info(MemoryPoolDebugCheckBlockOwnership GENZH_MEMORYPOOL_DEBUG_CHECK_BLOCK_OWNERSHIP "Build with Memory Pool block ownership checks")
add_feature_info(MemoryPoolDebugIntenseDmaBookkeeping GENZH_MEMORYPOOL_DEBUG_INTENSE_DMA_BOOKKEEPING "Build with Memory Pool intense DMA bookkeeping")


### INTERFACE LIBRARY SETUP ###

add_library(gz_config INTERFACE)

if(NOT IS_VS6_BUILD)
    # Because we set CMAKE_CXX_STANDARD_REQUIRED and CMAKE_CXX_EXTENSIONS in the compilers.cmake this should be enforced.
    target_compile_features(gz_config INTERFACE cxx_std_20)
endif()

target_compile_options(gz_config INTERFACE ${GENZH_FLAGS})

# This disables a lot of warnings steering developers to use windows only functions/function names.
if(MSVC)
    target_compile_definitions(gz_config INTERFACE _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS $<$<CONFIG:DEBUG>:_DEBUG_CRT>)
endif()

if(GENZH_BUILD_DEBUG)
    target_compile_definitions(gz_config INTERFACE _DEBUG WWDEBUG DEBUG)
else()
    target_compile_definitions(gz_config INTERFACE _RELEASE)

    if(GENZH_BUILD_INTERNAL)
        target_compile_definitions(gz_config INTERFACE _INTERNAL)
    endif()
    if(GENZH_BUILD_PROFILE)
        target_compile_definitions(gz_config INTERFACE _PROFILE)
    endif()
endif()


# Game Memory features
if(NOT GENZH_GAMEMEMORY_ENABLE)
    target_compile_definitions(gz_config INTERFACE DISABLE_GAMEMEMORY=1)
endif()

# Memory pool features
if(GENZH_MEMORYPOOL_OVERRIDE_MALLOC)
    target_compile_definitions(gz_config INTERFACE MEMORYPOOL_OVERRIDE_MALLOC=1)
endif()

if(NOT GENZH_MEMORYPOOL_MPSB_DLINK)
    target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_MPSB_DLINK=1)
endif()

# Memory pool debugs
if(NOT GENZH_MEMORYPOOL_DEBUG)
    target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_DEBUG=1)
else()
    if(NOT GENZH_MEMORYPOOL_DEBUG_CUSTOM_NEW)
        target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_DEBUG_CUSTOM_NEW=1)
    endif()
    
    if(GENZH_MEMORYPOOL_DEBUG_CHECKPOINTING)
        # Set to 0 to override the default setting in code
        target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_CHECKPOINTING=0)
    else()
        target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_CHECKPOINTING=1)
    endif()
    
    if(NOT GENZH_MEMORYPOOL_DEBUG_BOUNDINGWALL)
        target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_BOUNDINGWALL=1)
    endif()
    
    if(NOT GENZH_MEMORYPOOL_DEBUG_STACKTRACE)
        target_compile_definitions(gz_config INTERFACE DISABLE_MEMORYPOOL_STACKTRACE=1)
    endif()
    
    if(GENZH_MEMORYPOOL_DEBUG_INTENSE_VERIFY)
        target_compile_definitions(gz_config INTERFACE MEMORYPOOL_INTENSE_VERIFY=1)
    endif()
    
    if(GENZH_MEMORYPOOL_DEBUG_CHECK_BLOCK_OWNERSHIP)
        target_compile_definitions(gz_config INTERFACE MEMORYPOOL_CHECK_BLOCK_OWNERSHIP=1)
    endif()
    
    if(GENZH_MEMORYPOOL_DEBUG_INTENSE_DMA_BOOKKEEPING)
        target_compile_definitions(gz_config INTERFACE INTENSE_DMA_BOOKKEEPING=1)
    endif()
endif()
