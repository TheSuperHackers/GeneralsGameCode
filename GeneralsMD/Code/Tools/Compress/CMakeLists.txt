set(COMRPESS_SRC
    "Compress.cpp"
    "../CRCDiff/debug.cpp"
    "../CRCDiff/debug.h"
)

add_executable(z_compress WIN32)
set_target_properties(z_compress PROPERTIES OUTPUT_NAME compress)

target_sources(z_compress PRIVATE ${COMRPESS_SRC})

target_link_libraries(z_compress PRIVATE
    z_compression
    zi_libraries_include
    zi_libraries_source
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(MSVC_VERSION LESS 1300) # VS6
        target_compile_definitions(z_compress PRIVATE vsnprintf=_vsnprintf)
    endif()
    target_link_options(z_compress PRIVATE /subsystem:console)
endif()
