find_package(Tracy CONFIG QUIET)
if(NOT Tracy_FOUND)
    FetchContent_Declare(
        tracy
        EXCLUDE_FROM_ALL # Exclude Tracy's development files from the INSTALL target.
        GIT_REPOSITORY https://github.com/TheSuperHackers/tracy
        GIT_TAG        05cceee0df3b8d7c6fa87e9638af311dbabc63cb # 0.13.1
    )
    FetchContent_MakeAvailable(tracy)
endif()

if(NOT TARGET Tracy::TracyClient)
    message(FATAL_ERROR "Tracy is enabled but Tracy::TracyClient was not found.")
endif()

add_library(core_profile_tracy INTERFACE)
target_compile_definitions(core_profile_tracy INTERFACE
    RTS_PROFILE_TRACY
)
target_link_libraries(core_profile_tracy INTERFACE Tracy::TracyClient)
