find_package(Tracy CONFIG QUIET)
if(NOT Tracy_FOUND)
    set(TRACY_ENABLE ON CACHE BOOL "Enable Tracy profiling" FORCE)
    FetchContent_Declare(
        tracy
        EXCLUDE_FROM_ALL # Exclude Tracy's development files from the INSTALL target.
        GIT_REPOSITORY https://github.com/wolfpld/tracy
        GIT_TAG        v0.15.0
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
