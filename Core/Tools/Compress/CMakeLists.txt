set(COMRPESS_SRC
    "Compress.cpp"
)

add_executable(core_compress WIN32)
set_target_properties(core_compress PROPERTIES OUTPUT_NAME compress)

target_sources(core_compress PRIVATE ${COMRPESS_SRC})

target_link_libraries(core_compress PRIVATE
    core_config
    core_compression
    corei_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(core_compress PRIVATE /subsystem:console)
endif()
