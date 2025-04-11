set(VERSIONUPDATE_SRC
    "versionUpdate.cpp"
)

add_executable(z_versionupdate WIN32)
set_target_properties(z_versionupdate PROPERTIES OUTPUT_NAME versionupdate)

target_sources(z_versionupdate PRIVATE ${VERSIONUPDATE_SRC})

target_link_libraries(z_versionupdate PRIVATE
    core_config
    z_wwlib
)
