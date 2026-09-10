# An unattended game must not retain every profiling event while waiting for
# a capture connection. Apply this to the client and all instrumented callers,
# including existing build directories that cached the upstream OFF default.
set(TRACY_ON_DEMAND ON CACHE BOOL "Record Tracy events only while connected." FORCE)

find_package(Tracy CONFIG QUIET)
if(NOT Tracy_FOUND)
    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/TheSuperHackers/tracy
        GIT_TAG        05cceee0df3b8d7c6fa87e9638af311dbabc63cb # 0.13.1
    )
    FetchContent_MakeAvailable(tracy)
endif()

if(NOT TARGET TracyClient)
    message(FATAL_ERROR "Tracy is enabled but TracyClient was not found.")
endif()

get_target_property(tracy_client_definitions TracyClient INTERFACE_COMPILE_DEFINITIONS)
if(NOT "TRACY_ON_DEMAND" IN_LIST tracy_client_definitions)
    message(FATAL_ERROR "TracyClient must be built with TRACY_ON_DEMAND=ON. Rebuild the installed Tracy package or use the fetched client.")
endif()

target_compile_definitions(TracyClient INTERFACE
    RTS_PROFILE_TRACY
)

add_library(core_profile_tracy ALIAS TracyClient)
