set(PROFILE_SRC
    "internal_cmd.h"
    "internal_funclevel.h"
    "internal_highlevel.h"
    "internal_result.h"
    "internal.h"
    "profile_cmd.cpp"
    "profile_doc.h"
    "profile_funclevel.h"
    "profile_highlevel.h"
    "profile_result.h"
    "profile.h"
)

if(NOT APPLE)
    list(APPEND PROFILE_SRC
        "profile_funclevel.cpp"
        "profile_highlevel.cpp"
        "profile_result.cpp"
        "profile.cpp"
    )
endif()

add_library(core_profile STATIC)

target_sources(core_profile PRIVATE ${PROFILE_SRC})

target_include_directories(core_profile INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(core_profile PRIVATE
    [["Utility/CppMacros.h"]] # Must be first, to be removed when abandoning VC6
    <windows.h>
    "profile.h"
    "internal.h"
)

target_link_libraries(core_profile PRIVATE
    core_wwcommon
    corei_always
)
