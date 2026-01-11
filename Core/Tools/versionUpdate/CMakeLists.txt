set(VERSIONUPDATE_SRC
    "versionUpdate.cpp"
)

add_executable(core_versionupdate WIN32)
set_target_properties(core_versionupdate PROPERTIES OUTPUT_NAME versionupdate)

target_sources(core_versionupdate PRIVATE ${VERSIONUPDATE_SRC})

target_link_libraries(core_versionupdate PRIVATE
    core_config
    core_wwlib
    corei_always
)
