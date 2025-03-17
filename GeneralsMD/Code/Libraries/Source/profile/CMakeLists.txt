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
set_target_properties(z_profile PROPERTIES OUTPUT_NAME profile)

target_sources(z_profile PRIVATE ${PROFILE_SRC})

target_include_directories(z_profile INTERFACE
    .
)

target_link_libraries(z_profile PRIVATE
    zi_libraries_include
)

if(GENZH_BUILD_DEBUG)
    set_target_properties(z_profile PROPERTIES OUTPUT_NAME profiledebug)
elseif(GENZH_BUILD_INTERNAL)
    set_target_properties(z_profile PROPERTIES OUTPUT_NAME profileinternal)
else()
    set_target_properties(z_profile PROPERTIES OUTPUT_NAME profile)
endif()
