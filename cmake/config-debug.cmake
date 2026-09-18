set(RTS_DEBUG_LOGGING "DEFAULT" CACHE STRING "Enables debug logging. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_LOGGING PROPERTY STRINGS DEFAULT ON OFF)

set(RTS_DEBUG_CRASHING "DEFAULT" CACHE STRING "Enables debug assert dialogs. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_CRASHING PROPERTY STRINGS DEFAULT ON OFF)

set(RTS_DEBUG_STACKTRACE "DEFAULT" CACHE STRING "Enables debug stacktracing. This inherintly also enables debug logging. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_STACKTRACE PROPERTY STRINGS DEFAULT ON OFF)

set(RTS_DEBUG_PROFILE "DEFAULT" CACHE STRING "Enables debug profiling. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_PROFILE PROPERTY STRINGS DEFAULT ON OFF)

option(RTS_DEBUG_CHEATS "Enables debug cheats in release builds" OFF)
option(RTS_DEBUG_INCLUDE_DEBUG_LOG_IN_CRC_LOG "Includes normal debug log in crc log" OFF)
option(RTS_DEBUG_MULTI_INSTANCE "Enables multi client instance support" OFF)


define_tristate_option(RTS_DEBUG_LOGGING    DebugLogging    "Build with Debug Logging"      DEBUG_LOGGING    DISABLE_DEBUG_LOGGING)
define_tristate_option(RTS_DEBUG_CRASHING   DebugCrashing   "Build with Debug Crashing"     DEBUG_CRASHING   DISABLE_DEBUG_CRASHING)
define_tristate_option(RTS_DEBUG_STACKTRACE DebugStacktrace "Build with Debug Stacktracing" DEBUG_STACKTRACE DISABLE_DEBUG_STACKTRACE)
define_tristate_option(RTS_DEBUG_PROFILE    DebugProfile    "Build with Debug Profiling"    DEBUG_PROFILE    DISABLE_DEBUG_PROFILE)

add_feature_info(DebugCheats RTS_DEBUG_CHEATS "Build with Debug Cheats in release builds")
add_feature_info(DebugIncludeDebugLogInCrcLog RTS_DEBUG_INCLUDE_DEBUG_LOG_IN_CRC_LOG "Build with Debug Logging in CRC log")
add_feature_info(DebugMultiInstance RTS_DEBUG_MULTI_INSTANCE "Build with Multi Client Instance support")


if(RTS_DEBUG_CHEATS)
    target_compile_definitions(core_config INTERFACE _ALLOW_DEBUG_CHEATS_IN_RELEASE)
endif()

if(RTS_DEBUG_INCLUDE_DEBUG_LOG_IN_CRC_LOG)
    target_compile_definitions(core_config INTERFACE INCLUDE_DEBUG_LOG_IN_CRC_LOG)
endif()

if(RTS_DEBUG_MULTI_INSTANCE)
    target_compile_definitions(core_config INTERFACE RTS_MULTI_INSTANCE)
endif()
