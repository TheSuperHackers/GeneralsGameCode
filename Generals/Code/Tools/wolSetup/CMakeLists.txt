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
    "WOLAPI/wolapi.h"
    "WOLAPI/wolapi_i.c"
)

add_executable(g_wolsetup WIN32)
set_target_properties(g_wolsetup PROPERTIES OUTPUT_NAME wolsetup)

target_sources(g_wolsetup PRIVATE ${WOLSETUP_SRC})

target_link_libraries(g_wolsetup PRIVATE
    gz_config
    Version
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(g_wolsetup PRIVATE wolSetup.rc)
endif()

# TODO: Copy the dll's that reside in the WOLAPI folder.
