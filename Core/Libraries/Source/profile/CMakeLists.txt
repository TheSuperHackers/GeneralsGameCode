set(PROFILE_SRC
    "_pch.cpp"
    "_pch.h"
    "internal_cmd.h"
    "internal_funclevel.h"
    "internal_highlevel.h"
    "internal_result.h"
    "internal.h"
    "profile_cmd.cpp"
    "profile_doc.h"
    "profile_funclevel.cpp"
    "profile_funclevel.h"
    "profile_highlevel.cpp"
    "profile_highlevel.h"
    "profile_result.cpp"
    "profile_result.h"
    "profile.cpp"
    "profile.h"
)

add_library(core_profile STATIC)

target_sources(core_profile PRIVATE ${PROFILE_SRC})

target_include_directories(core_profile INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_profile PRIVATE
    core_wwcommon
    core_config
    corei_always
)
