set(COMRPESS_SRC
    "Compress.cpp"
)

add_executable(z_compress WIN32)
set_target_properties(z_compress PROPERTIES OUTPUT_NAME compress)

target_sources(z_compress PRIVATE ${COMRPESS_SRC})

target_link_libraries(z_compress PRIVATE
    core_config
    z_compression
    zi_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(IS_VS6_BUILD)
        target_compile_definitions(z_compress PRIVATE vsnprintf=_vsnprintf)
    endif()
    target_link_options(z_compress PRIVATE /subsystem:console)
endif()
