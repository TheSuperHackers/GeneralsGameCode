set(ASSETCULL_SRC
    "assetcull.cpp"
)

add_executable(core_assetcull WIN32)
set_target_properties(core_assetcull PROPERTIES OUTPUT_NAME assetcull)

target_sources(core_assetcull PRIVATE ${ASSETCULL_SRC})

target_link_libraries(core_assetcull PRIVATE
    corei_always
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_options(core_assetcull PRIVATE /subsystem:console)
endif()
