set(COMRPESS_SRC
    "Compress.cpp"
    "../CRCDiff/debug.cpp"
    "../CRCDiff/debug.h"
)

add_executable(g_compress WIN32)
set_target_properties(g_compress PROPERTIES OUTPUT_NAME compress)

target_sources(g_compress PRIVATE ${COMRPESS_SRC})

target_link_libraries(g_compress PRIVATE
    g_compression
    gi_libraries_include
    gi_libraries_source
    gz_config
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(IS_VS6_BUILD)
        target_compile_definitions(g_compress PRIVATE vsnprintf=_vsnprintf)
    endif()
    target_link_options(g_compress PRIVATE /subsystem:console)
endif()
