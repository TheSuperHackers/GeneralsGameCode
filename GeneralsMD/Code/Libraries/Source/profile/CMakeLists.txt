set(PROFILE_SRC
    "profile_funclevel.cpp"
    "profile_funclevel.h"
    "profile_highlevel.cpp"
    "profile_highlevel.h"
    "profile_result.cpp"
    "profile_result.h"
    "_pch.cpp"
    "_pch.h"
    "internal.h"
    "internal_cmd.h"
    "internal_funclevel.h"
    "internal_highlevel.h"
    "internal_result.h"
    "profile.cpp"
    "profile.h"
    "profile_cmd.cpp"
    "profile_doc.h"
)

add_library(z_profile STATIC)

target_sources(z_profile PRIVATE ${PROFILE_SRC})

target_include_directories(z_profile INTERFACE
    .
)

target_link_libraries(z_profile PRIVATE
    gz_config
    zi_always
)
