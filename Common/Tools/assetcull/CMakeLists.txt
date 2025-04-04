set(ASSETCULL_SRC
    "assetcull.cpp"
)

add_executable(gz_assetcull WIN32)
set_target_properties(gz_assetcull PROPERTIES OUTPUT_NAME assetcull)

target_sources(gz_assetcull PRIVATE ${ASSETCULL_SRC})

target_link_libraries(gz_assetcull PRIVATE
    gz_config
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(gz_assetcull PRIVATE /subsystem:console)
endif()
