set(COMRPESS_SRC
    "Compress.cpp"
    "../CRCDiff/debug.cpp"
    "../CRCDiff/debug.h"
)

add_executable(gz_compress WIN32)
set_target_properties(gz_compress PROPERTIES OUTPUT_NAME compress)

target_sources(gz_compress PRIVATE ${COMRPESS_SRC})

target_link_libraries(gz_compress PRIVATE
    gz_config
    gz_compression
    ci_libraries_include
    ci_libraries_source
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(IS_VS6_BUILD)
        target_compile_definitions(gz_compress PRIVATE vsnprintf=_vsnprintf)
    endif()
    target_link_options(gz_compress PRIVATE /subsystem:console)
endif()
