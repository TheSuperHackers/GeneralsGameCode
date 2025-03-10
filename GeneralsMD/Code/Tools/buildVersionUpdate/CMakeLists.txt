set(BUILDVERSIONUPDATE_SRC
    "buildVersionUpdate.cpp"
)

add_executable(buildversionupdate WIN32)

target_sources(buildversionupdate PRIVATE ${BUILDVERSIONUPDATE_SRC})
