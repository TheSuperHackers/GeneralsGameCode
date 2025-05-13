# Game Memory features
option(RTS_GAMEMEMORY_ENABLE "Enables the memory pool and dynamic memory allocator." ON)

# Disable Game Memory if ASAN is enabled - Game Memory overrides new/delete and interferes with ASAN
if(RTS_BUILD_OPTION_ASAN)
    set(RTS_GAMEMEMORY_ENABLE OFF)
endif()

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
