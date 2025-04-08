### BUILD OPTIONS ###

# Do we want to build extra SDK stuff or just the game binary?
option(RTS_BUILD_ZEROHOUR "Build Zero Hour code." ON)
option(RTS_BUILD_GENERALS "Build Generals code." ON)
option(RTS_BUILD_OPTION_INTERNAL "Build code with the \"Internal\" configuration." OFF)
option(RTS_BUILD_OPTION_PROFILE "Build code with the \"Profile\" configuration." OFF)
option(RTS_BUILD_OPTION_DEBUG "Build code with the \"Debug\" configuration." OFF)
option(RTS_BUILD_OPTION_ASAN "Build code with Address Sanitizer." OFF)

if(NOT RTS_BUILD_ZEROHOUR AND NOT RTS_BUILD_GENERALS)
    set(RTS_BUILD_ZEROHOUR TRUE)
    message("You must select one project to build, building Zero Hour by default.")
endif()

add_feature_info(ZeroHourStuff RTS_BUILD_ZEROHOUR "Build Zero Hour code")
add_feature_info(GeneralsStuff RTS_BUILD_GENERALS "Build Generals code")
add_feature_info(InternalBuild RTS_BUILD_OPTION_INTERNAL "Building as an \"Internal\" build")
add_feature_info(ProfileBuild RTS_BUILD_OPTION_PROFILE "Building as a \"Profile\" build")
add_feature_info(DebugBuild RTS_BUILD_OPTION_DEBUG "Building as a \"Debug\" build")
add_feature_info(AddressSanitizer RTS_BUILD_OPTION_ASAN "Building with address sanitizer")


### GAME MEMORY OPTIONS ###

# Game Memory features
option(RTS_GAMEMEMORY_ENABLE "Enables the memory pool and dynamic memory allocator." ON)

# Memory pool features
option(RTS_MEMORYPOOL_OVERRIDE_MALLOC "Enables the Dynamic Memory Allocator for malloc calls." OFF)
option(RTS_MEMORYPOOL_MPSB_DLINK "Adds a backlink to MemoryPoolSingleBlock. Makes it faster to free raw DMA blocks, but increases memory consumption." ON)

# Memory pool debugs
option(RTS_MEMORYPOOL_DEBUG "Enables Memory Pool debug." ON)
option(RTS_MEMORYPOOL_DEBUG_CUSTOM_NEW "Enables a custom new operator for the Memory Pool." ON)
option(RTS_MEMORYPOOL_DEBUG_CHECKPOINTING "Records checkpoint information about the history of memory allocations." OFF)
option(RTS_MEMORYPOOL_DEBUG_BOUNDINGWALL "Enables bounding wall checks around memory chunks to find memory trampling." ON)
option(RTS_MEMORYPOOL_DEBUG_STACKTRACE "Enables stack trace collection for allocations. Reduces runtime performance significantly." OFF)
option(RTS_MEMORYPOOL_DEBUG_INTENSE_VERIFY "Enables intensive verifications after nearly every memory operation. OFF by default, since it slows down things a lot, but is worth turning on for really obscure memory corruption issues." OFF)
option(RTS_MEMORYPOOL_DEBUG_CHECK_BLOCK_OWNERSHIP "Enables debug to verify that a block actually belongs to the pool it is called with. This is great for debugging, but can be realllly slow, so is OFF by default." OFF)
option(RTS_MEMORYPOOL_DEBUG_INTENSE_DMA_BOOKKEEPING "Prints statistics for memory usage of Memory Pools." OFF)

# Game Memory features
add_feature_info(GameMemoryEnable RTS_GAMEMEMORY_ENABLE "Build with the original game memory implementation")

# Memory pool features
add_feature_info(MemoryPoolOverrideMalloc RTS_MEMORYPOOL_OVERRIDE_MALLOC "Build with Memory Pool malloc")
add_feature_info(MemoryPoolMpsbDlink RTS_MEMORYPOOL_MPSB_DLINK "Build with Memory Pool backlink")

# Memory pool debugs
add_feature_info(MemoryPoolDebug RTS_MEMORYPOOL_DEBUG "Build with Memory Pool debug")
add_feature_info(MemoryPoolDebugCustomNew RTS_MEMORYPOOL_DEBUG_CUSTOM_NEW "Build with Memory Pool custom new")
add_feature_info(MemoryPoolDebugCheckpointing RTS_MEMORYPOOL_DEBUG_CHECKPOINTING "Build with Memory Pool checkpointing")
add_feature_info(MemoryPoolDebugBoundingwall RTS_MEMORYPOOL_DEBUG_BOUNDINGWALL "Build with Memory Pool Bounding Wall")
add_feature_info(MemoryPoolDebugStacktrace RTS_MEMORYPOOL_DEBUG_STACKTRACE "Build with Memory Pool Stacktrace")
add_feature_info(MemoryPoolDebugIntenseVerify RTS_MEMORYPOOL_DEBUG_INTENSE_VERIFY "Build with Memory Pool intense verify")
add_feature_info(MemoryPoolDebugCheckBlockOwnership RTS_MEMORYPOOL_DEBUG_CHECK_BLOCK_OWNERSHIP "Build with Memory Pool block ownership checks")
add_feature_info(MemoryPoolDebugIntenseDmaBookkeeping RTS_MEMORYPOOL_DEBUG_INTENSE_DMA_BOOKKEEPING "Build with Memory Pool intense DMA bookkeeping")


### INTERFACE LIBRARY SETUP ###

add_library(core_config INTERFACE)

if(NOT IS_VS6_BUILD)
    # Because we set CMAKE_CXX_STANDARD_REQUIRED and CMAKE_CXX_EXTENSIONS in the compilers.cmake this should be enforced.
    target_compile_features(core_config INTERFACE cxx_std_20)
endif()

target_compile_options(core_config INTERFACE ${RTS_FLAGS})

# This disables a lot of warnings steering developers to use windows only functions/function names.
if(MSVC)
    target_compile_definitions(core_config INTERFACE _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS $<$<CONFIG:DEBUG>:_DEBUG_CRT>)
endif()

if(RTS_BUILD_OPTION_DEBUG)
    target_compile_definitions(core_config INTERFACE _DEBUG WWDEBUG DEBUG)
else()
    target_compile_definitions(core_config INTERFACE _RELEASE)

    if(RTS_BUILD_OPTION_INTERNAL)
        target_compile_definitions(core_config INTERFACE _INTERNAL)
    endif()
    if(RTS_BUILD_OPTION_PROFILE)
        target_compile_definitions(core_config INTERFACE _PROFILE)
    endif()
endif()


# Game Memory features
if(NOT RTS_GAMEMEMORY_ENABLE)
    target_compile_definitions(core_config INTERFACE DISABLE_GAMEMEMORY=1)
endif()

# Memory pool features
if(RTS_MEMORYPOOL_OVERRIDE_MALLOC)
    target_compile_definitions(core_config INTERFACE MEMORYPOOL_OVERRIDE_MALLOC=1)
endif()

if(NOT RTS_MEMORYPOOL_MPSB_DLINK)
    target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_MPSB_DLINK=1)
endif()

# Memory pool debugs
if(NOT RTS_MEMORYPOOL_DEBUG)
    target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_DEBUG=1)
else()
    if(NOT RTS_MEMORYPOOL_DEBUG_CUSTOM_NEW)
        target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_DEBUG_CUSTOM_NEW=1)
    endif()
    
    if(RTS_MEMORYPOOL_DEBUG_CHECKPOINTING)
        # Set to 0 to override the default setting in code
        target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_CHECKPOINTING=0)
    else()
        target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_CHECKPOINTING=1)
    endif()
    
    if(NOT RTS_MEMORYPOOL_DEBUG_BOUNDINGWALL)
        target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_BOUNDINGWALL=1)
    endif()
    
    if(NOT RTS_MEMORYPOOL_DEBUG_STACKTRACE)
        target_compile_definitions(core_config INTERFACE DISABLE_MEMORYPOOL_STACKTRACE=1)
    endif()
    
    if(RTS_MEMORYPOOL_DEBUG_INTENSE_VERIFY)
        target_compile_definitions(core_config INTERFACE MEMORYPOOL_INTENSE_VERIFY=1)
    endif()
    
    if(RTS_MEMORYPOOL_DEBUG_CHECK_BLOCK_OWNERSHIP)
        target_compile_definitions(core_config INTERFACE MEMORYPOOL_CHECK_BLOCK_OWNERSHIP=1)
    endif()
    
    if(RTS_MEMORYPOOL_DEBUG_INTENSE_DMA_BOOKKEEPING)
        target_compile_definitions(core_config INTERFACE INTENSE_DMA_BOOKKEEPING=1)
    endif()
endif()
