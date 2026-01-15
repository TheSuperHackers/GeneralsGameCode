set(WOLSETUP_SRC
    "resource.h"
    "StdAfx.cpp"
    "StdAfx.h"
    "verchk.cpp"
    "verchk.h"
    "wolInit.cpp"
    "wolSetup.cpp"
    "wolSetup.h"
    "WOLAPI/chatdefs.h"
    "WOLAPI/downloaddefs.h"
    "WOLAPI/ftpdefs.h"
    "WOLAPI/netutildefs.h"
    "WOLAPI/wolapi_i.c"
    "WOLAPI/wolapi.h"
)

add_executable(core_wolsetup WIN32)
set_target_properties(core_wolsetup PROPERTIES OUTPUT_NAME wolsetup)

target_sources(core_wolsetup PRIVATE ${WOLSETUP_SRC})

target_link_libraries(core_wolsetup PRIVATE
    core_config
    corei_always
    Version
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(core_wolsetup PRIVATE wolSetup.rc)
endif()

# TODO: Copy the dll's that reside in the WOLAPI folder.
