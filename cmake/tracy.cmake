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

target_compile_definitions(TracyClient INTERFACE
    RTS_PROFILE_TRACY
)

add_library(core_profile_tracy ALIAS TracyClient)
