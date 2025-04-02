set(BUILDVERSIONUPDATE_SRC
    "buildVersionUpdate.cpp"
)

add_executable(gz_buildversionupdate WIN32)
set_target_properties(gz_buildversionupdate PROPERTIES OUTPUT_NAME buildversionupdate)

target_sources(gz_buildversionupdate PRIVATE ${BUILDVERSIONUPDATE_SRC})

target_link_libraries(gz_buildversionupdate PRIVATE
    gz_config
)
