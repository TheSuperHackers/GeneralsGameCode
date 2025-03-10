set(VERSIONUPDATE_SRC
    "versionUpdate.cpp"
)

add_executable(versionupdate WIN32)

target_sources(versionupdate PRIVATE ${VERSIONUPDATE_SRC})
