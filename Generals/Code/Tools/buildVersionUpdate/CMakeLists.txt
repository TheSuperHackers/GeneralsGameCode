set(BUILDVERSIONUPDATE_SRC
    "buildVersionUpdate.cpp"
)

add_executable(g_buildversionupdate WIN32)
set_target_properties(g_buildversionupdate PROPERTIES OUTPUT_NAME buildversionupdate)

target_sources(g_buildversionupdate PRIVATE ${BUILDVERSIONUPDATE_SRC})
