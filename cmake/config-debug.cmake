set(RTS_DEBUG_LOGGING "DEFAULT" CACHE STRING "Enables debug logging. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_LOGGING PROPERTY STRINGS DEFAULT ON OFF)

set(RTS_DEBUG_CRASHING "DEFAULT" CACHE STRING "Enables debug assert dialogs. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_CRASHING PROPERTY STRINGS DEFAULT ON OFF)

set(RTS_DEBUG_STACKTRACE "DEFAULT" CACHE STRING "Enables debug stacktracing. This inherintly also enables debug logging. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_STACKTRACE PROPERTY STRINGS DEFAULT ON OFF)

set(RTS_DEBUG_PROFILE "DEFAULT" CACHE STRING "Enables debug profiling. When DEFAULT, this option is enabled with DEBUG or INTERNAL")
set_property(CACHE RTS_DEBUG_PROFILE PROPERTY STRINGS DEFAULT ON OFF)


add_feature_info(DebugLogging RTS_DEBUG_LOGGING "Build with Debug Logging")
add_feature_info(DebugLogging RTS_DEBUG_CRASHING "Build with Debug Crashing")
add_feature_info(DebugLogging RTS_DEBUG_STACKTRACE "Build with Debug Stacktracing")
add_feature_info(DebugLogging RTS_DEBUG_PROFILE "Build with Debug Profiling")


macro(define_debug_option OptionName OptionEnabledCompileDef OptionDisabledCompileDef)
    if(${OptionName} STREQUAL "DEFAULT")
        # Does nothing
    elseif(${OptionName} STREQUAL "ON")
        target_compile_definitions(core_config INTERFACE ${OptionEnabledCompileDef}=1)
    elseif(${OptionName} STREQUAL "OFF")
        target_compile_definitions(core_config INTERFACE ${OptionDisabledCompileDef}=1)
    else()
        message(FATAL_ERROR "Unhandled option")
    endif()
endmacro()


define_debug_option(RTS_DEBUG_LOGGING    DEBUG_LOGGING    DISABLE_DEBUG_LOGGING   )
define_debug_option(RTS_DEBUG_CRASHING   DEBUG_CRASHING   DISABLE_DEBUG_CRASHING  )
define_debug_option(RTS_DEBUG_STACKTRACE DEBUG_STACKTRACE DISABLE_DEBUG_STACKTRACE)
define_debug_option(RTS_DEBUG_PROFILE    DEBUG_PROFILE    DISABLE_DEBUG_PROFILE   )
