set(VERSIONUPDATE_SRC
    "versionUpdate.cpp"
)

add_executable(g_versionupdate WIN32)
set_target_properties(g_versionupdate PROPERTIES OUTPUT_NAME versionupdate)

target_sources(g_versionupdate PRIVATE ${VERSIONUPDATE_SRC})
