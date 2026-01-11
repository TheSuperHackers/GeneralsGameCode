set(BUILDVERSIONUPDATE_SRC
    "buildVersionUpdate.cpp"
)

add_executable(core_buildversionupdate WIN32)
set_target_properties(core_buildversionupdate PROPERTIES OUTPUT_NAME buildversionupdate)

target_sources(core_buildversionupdate PRIVATE ${BUILDVERSIONUPDATE_SRC})

target_link_libraries(core_buildversionupdate PRIVATE
    core_config
    core_wwlib
    corei_always
)
