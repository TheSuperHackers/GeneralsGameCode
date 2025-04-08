set(ASSETCULL_SRC
    "assetcull.cpp"
)

add_executable(z_assetcull WIN32)
set_target_properties(z_assetcull PROPERTIES OUTPUT_NAME assetcull)

target_sources(z_assetcull PRIVATE ${ASSETCULL_SRC})

target_link_libraries(z_assetcull PRIVATE
    core_config
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(z_assetcull PRIVATE /subsystem:console)
endif()
